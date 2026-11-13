// SPDX-License-Identifier: CC0-1.0
// 参考: https://nyaannyaan.github.io/library/misc/mo.hpp.html
// 区間移動とblock順序の数学的構成を参考に独立実装。更新版は時刻の往復を加える。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/mo.md
// API一覧
// Mo(n=0): 長さn>=0の配列上の静的Mo。構築O(1)。add_query(l,r)は半開区間を記録し安定な要求IDを返す。
// MoWithUpdates(n=0): 構築O(1)。add_update(pos)が変更IDを返す。値と旧値は利用側が保持する。
//   add_query(l,r)は現在の変更数の時刻、add_query(l,r,time)は記録済みの任意時刻で照会を記録。
// QueryはMoでは{left,right}、更新版では{left,right,time}。query(id)は値、queries()はconst span。
// 更新版のupdate_position(id)/updates()は変更位置/そのconst span。追加は償却O(1)。確保失敗時内容維持。
// 共通: length()/query_count()/empty()は配列長/要求数/要求なし。更新版update_count()は変更数。
// block_size(b=0): 実際の幅B>0。正のbはその値、0は自動（静的n/max(1,floor(sqrt(Q)))、更新版floor(cbrt(n^2)))。
//   自動幅は1以上、n=0は1。整数演算で計算する。静的O(log(Q+1))、更新版O(log(n+1))。
// order(b=0): 処理順の要求ID vector。静的はleft block→right、更新版はleft/right block→timeで蛇行。
//   全sortキーが同じならID順。ID自体は追加順から変えない。
//   O(Q log(Q+1)+log(n+Q+1))時間・O(Q)作業領域。
// Mo.run(add_left,add_right,erase_left,erase_right,answer,b=0): 各伸縮でindex、answerで要求IDをstd::invoke。
//   run(add,erase,answer,b=0)は左右対称版。戻り値Window{left,right}は最後の区間、要求なしは{0,0}。
// 更新版.run(add_left,add_right,erase_left,erase_right,apply,undo,answer,b=0):
//   apply/undo(change_id,left,right)で1時刻進退し、その後区間を移動する。run(add,erase,apply,undo,answer,b=0)も可。
//   戻り値State{left,right,time}。要求なしは{0,0,0}。
// 外部の初期集計は空、更新版の値は時刻0。最初の空区間は最初に処理する要求のleftに置く。
// 終了時の外部状態は戻り値の区間/時刻のまま。再実行前に利用側で初期状態へ戻す。
// callbackはlvalueとして呼べること。callback中に本体を変更/移動しない。例外時に本体記録/viewは維持するが外部状態は戻さない。
// 計算用領域はcallback前に確保する。N=n,Q=照会数,U=変更数,K=1+floor(N/B)として、sort以外の移動回数は
//   静的O(QB+NK+Q)、更新版O(QB+NK+UK^2+Q)。callback自身の費用は別。自動幅が最速とは保証しない。
// reserve_queries(cap)、更新版reserve_updates(cap): 記録容量を予約。clear_requests(): nを保ち全記録を消す。
// reset(n=0)/clear(): 全初期化/長さ0。copyはO(Q+U)で独立・代入強保証。move元は長さ0/記録なし、swap O(1)。
// 形・件数・要求/変更の取得はO(1)。reserveは再確保時O(Q)/O(U)。reset/clearは記録の破棄を含む。
// viewは追加/容量変更/clear/代入/所有権移動まで有効。番号/負数/区間/時刻/負の幅の検査はSKIP_RUNTIME_CHECKSで除外。
// 容量保護、幅0の自動選択、整数計算の0除算回避は検査省略時も維持。

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_MO_ORDERING_HELPERS_DEFINED
#define SIRKIT_MO_ORDERING_HELPERS_DEFINED
namespace reinsirk::detail {
struct MoOrderEntry { std::size_t id; std::int32_t left_block, right; };
struct TimedMoOrderEntry { std::size_t id, time; std::int32_t left_block, right_block; };
inline std::size_t mo_integer_sqrt(std::size_t value) noexcept {
    std::size_t low = 0, high = value;
    while (low < high) {
        const auto distance = high - low, middle = low + distance / 2 + distance % 2;
        if (middle <= value / middle) low = middle; else high = middle - 1;
    }
    return low;
}
inline std::int32_t mo_block_width(std::int32_t n, std::size_t queries, std::int32_t requested, bool timed) {
#ifndef SKIP_RUNTIME_CHECKS
    if (requested < 0) throw std::invalid_argument("Mo block size");
#endif
    if (requested > 0) return requested;
    if (n <= 0) return 1;
    if (!timed) return static_cast<std::int32_t>(std::max(std::size_t{1}, static_cast<std::size_t>(n) / std::max(std::size_t{1}, mo_integer_sqrt(queries))));
    // int32_tの正の長さの二乗はuint64_tに収まる。cubeは除算で比較する。
    const auto square = static_cast<std::uint64_t>(n) * static_cast<std::uint64_t>(n);
    std::int32_t low = 1, high = n;
    while (low < high) {
        const auto distance = high - low, middle = low + distance / 2 + distance % 2;
        const auto candidate = static_cast<std::uint64_t>(middle);
        if (candidate <= square / candidate / candidate) low = middle; else high = middle - 1;
    }
    return low;
}
inline void mo_check_range(std::int32_t n, std::int32_t left, std::int32_t right) {
#ifndef SKIP_RUNTIME_CHECKS
    if (left < 0 || left > right || right > n) throw std::out_of_range("Mo range");
#else
    (void)n; (void)left; (void)right;
#endif
}
}
#endif

#ifndef SIRKIT_MO_DEFINED
#define SIRKIT_MO_DEFINED
class Mo {
public:
    struct Query { std::int32_t left, right; bool operator==(const Query&) const = default; };
    struct Window { std::int32_t left, right; bool operator==(const Window&) const = default; };
private:
    std::int32_t length_ = 0;
    std::vector<Query> queries_;
    std::vector<reinsirk::detail::MoOrderEntry> ordered_entries(std::int32_t requested) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (requested < 0) throw std::invalid_argument("Mo block size");
#endif
        std::vector<reinsirk::detail::MoOrderEntry> result;
        if (queries_.empty()) return result;
        const auto width = block_size(requested); result.reserve(queries_.size());
        for (std::size_t i = 0; i < queries_.size(); ++i) result.push_back({i, queries_[i].left / width, queries_[i].right});
        std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
            if (a.left_block != b.left_block) return a.left_block < b.left_block;
            if (a.right != b.right) return (a.left_block & 1) ? a.right > b.right : a.right < b.right;
            return a.id < b.id;
        });
        return result;
    }
public:
    explicit Mo(std::int32_t n = 0) : length_(n) {
#ifndef SKIP_RUNTIME_CHECKS
        if (n < 0) throw std::invalid_argument("Mo length");
#endif
    }
    Mo(const Mo&) = default;
    Mo& operator=(const Mo& other) { if (this != &other) { Mo copy(other); swap(copy); } return *this; }
    Mo(Mo&& other) noexcept { swap(other); }
    Mo& operator=(Mo&& other) noexcept { if (this != &other) { clear(); swap(other); } return *this; }
    void swap(Mo& other) noexcept { std::swap(length_, other.length_); queries_.swap(other.queries_); }
    friend void swap(Mo& a, Mo& b) noexcept { a.swap(b); }
    std::int32_t length() const noexcept { return length_; }
    std::size_t query_count() const noexcept { return queries_.size(); }
    bool empty() const noexcept { return queries_.empty(); }
    std::span<const Query> queries() const noexcept { return queries_; }
    Query query(std::size_t id) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= queries_.size()) throw std::out_of_range("Mo query ID");
#endif
        return queries_[id];
    }
    std::size_t add_query(std::int32_t left, std::int32_t right) {
        reinsirk::detail::mo_check_range(length_, left, right); const auto id = queries_.size(); queries_.push_back({left, right}); return id;
    }
    std::int32_t block_size(std::int32_t requested = 0) const { return reinsirk::detail::mo_block_width(length_, queries_.size(), requested, false); }
    std::vector<std::size_t> order(std::int32_t requested = 0) const {
        const auto entries = ordered_entries(requested); std::vector<std::size_t> result; result.reserve(entries.size());
        for (const auto& entry : entries) result.push_back(entry.id);
        return result;
    }
    template<class AddLeft, class AddRight, class EraseLeft, class EraseRight, class Answer>
    Window run(AddLeft&& add_left, AddRight&& add_right, EraseLeft&& erase_left, EraseRight&& erase_right, Answer&& answer, std::int32_t requested = 0) const {
        const auto entries = ordered_entries(requested); if (entries.empty()) return {0, 0};
        auto left = queries_[entries.front().id].left, right = left;
        for (const auto& entry : entries) {
            const auto q = queries_[entry.id];
            while (left > q.left) std::invoke(add_left, --left);
            while (right < q.right) { std::invoke(add_right, right); ++right; }
            while (left < q.left) { std::invoke(erase_left, left); ++left; }
            while (right > q.right) std::invoke(erase_right, --right);
            std::invoke(answer, entry.id);
        }
        return {left, right};
    }
    template<class Add, class Erase, class Answer>
    Window run(Add&& add, Erase&& erase, Answer&& answer, std::int32_t requested = 0) const { return run(add, add, erase, erase, answer, requested); }
    void reserve_queries(std::size_t capacity) { queries_.reserve(capacity); }
    void clear_requests() noexcept { queries_.clear(); }
    void reset(std::int32_t n = 0) { Mo next(n); swap(next); }
    void clear() noexcept { queries_.clear(); length_ = 0; }
};
#endif

#ifndef SIRKIT_MO_WITH_UPDATES_DEFINED
#define SIRKIT_MO_WITH_UPDATES_DEFINED
class MoWithUpdates {
public:
    struct Query { std::int32_t left, right; std::size_t time; bool operator==(const Query&) const = default; };
    struct State { std::int32_t left, right; std::size_t time; bool operator==(const State&) const = default; };
private:
    std::int32_t length_ = 0;
    std::vector<Query> queries_;
    std::vector<std::int32_t> updates_;
    std::vector<reinsirk::detail::TimedMoOrderEntry> ordered_entries(std::int32_t requested) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (requested < 0) throw std::invalid_argument("Mo block size");
#endif
        std::vector<reinsirk::detail::TimedMoOrderEntry> result;
        if (queries_.empty()) return result;
        const auto width = block_size(requested); result.reserve(queries_.size());
        for (std::size_t i = 0; i < queries_.size(); ++i) result.push_back({i, queries_[i].time, queries_[i].left / width, queries_[i].right / width});
        std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
            if (a.left_block != b.left_block) return a.left_block < b.left_block;
            if (a.right_block != b.right_block) return (a.left_block & 1) ? a.right_block > b.right_block : a.right_block < b.right_block;
            if (a.time != b.time) return ((a.left_block ^ a.right_block) & 1) ? a.time > b.time : a.time < b.time;
            return a.id < b.id;
        });
        return result;
    }
public:
    explicit MoWithUpdates(std::int32_t n = 0) : length_(n) {
#ifndef SKIP_RUNTIME_CHECKS
        if (n < 0) throw std::invalid_argument("Mo length");
#endif
    }
    MoWithUpdates(const MoWithUpdates&) = default;
    MoWithUpdates& operator=(const MoWithUpdates& other) { if (this != &other) { MoWithUpdates copy(other); swap(copy); } return *this; }
    MoWithUpdates(MoWithUpdates&& other) noexcept { swap(other); }
    MoWithUpdates& operator=(MoWithUpdates&& other) noexcept { if (this != &other) { clear(); swap(other); } return *this; }
    void swap(MoWithUpdates& other) noexcept { std::swap(length_, other.length_); queries_.swap(other.queries_); updates_.swap(other.updates_); }
    friend void swap(MoWithUpdates& a, MoWithUpdates& b) noexcept { a.swap(b); }
    std::int32_t length() const noexcept { return length_; }
    std::size_t query_count() const noexcept { return queries_.size(); }
    std::size_t update_count() const noexcept { return updates_.size(); }
    bool empty() const noexcept { return queries_.empty(); }
    std::span<const Query> queries() const noexcept { return queries_; }
    std::span<const std::int32_t> updates() const noexcept { return updates_; }
    Query query(std::size_t id) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= queries_.size()) throw std::out_of_range("Mo query ID");
#endif
        return queries_[id];
    }
    std::int32_t update_position(std::size_t id) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= updates_.size()) throw std::out_of_range("Mo update ID");
#endif
        return updates_[id];
    }
    std::size_t add_update(std::int32_t position) {
#ifndef SKIP_RUNTIME_CHECKS
        if (position < 0 || position >= length_) throw std::out_of_range("Mo update position");
#endif
        const auto id = updates_.size(); updates_.push_back(position); return id;
    }
    std::size_t add_query(std::int32_t left, std::int32_t right) { return add_query(left, right, updates_.size()); }
    std::size_t add_query(std::int32_t left, std::int32_t right, std::size_t time) {
        reinsirk::detail::mo_check_range(length_, left, right);
#ifndef SKIP_RUNTIME_CHECKS
        if (time > updates_.size()) throw std::out_of_range("Mo query time");
#endif
        const auto id = queries_.size(); queries_.push_back({left, right, time}); return id;
    }
    std::int32_t block_size(std::int32_t requested = 0) const { return reinsirk::detail::mo_block_width(length_, queries_.size(), requested, true); }
    std::vector<std::size_t> order(std::int32_t requested = 0) const {
        const auto entries = ordered_entries(requested); std::vector<std::size_t> result; result.reserve(entries.size());
        for (const auto& entry : entries) result.push_back(entry.id);
        return result;
    }
    template<class AddLeft, class AddRight, class EraseLeft, class EraseRight, class Apply, class Undo, class Answer>
    State run(AddLeft&& add_left, AddRight&& add_right, EraseLeft&& erase_left, EraseRight&& erase_right,
              Apply&& apply, Undo&& undo, Answer&& answer, std::int32_t requested = 0) const {
        const auto entries = ordered_entries(requested); if (entries.empty()) return {0, 0, 0};
        auto left = queries_[entries.front().id].left, right = left; std::size_t time = 0;
        for (const auto& entry : entries) {
            const auto q = queries_[entry.id];
            while (time < q.time) { std::invoke(apply, time, left, right); ++time; }
            while (time > q.time) std::invoke(undo, --time, left, right);
            while (left > q.left) std::invoke(add_left, --left);
            while (right < q.right) { std::invoke(add_right, right); ++right; }
            while (left < q.left) { std::invoke(erase_left, left); ++left; }
            while (right > q.right) std::invoke(erase_right, --right);
            std::invoke(answer, entry.id);
        }
        return {left, right, time};
    }
    template<class Add, class Erase, class Apply, class Undo, class Answer>
    State run(Add&& add, Erase&& erase, Apply&& apply, Undo&& undo, Answer&& answer, std::int32_t requested = 0) const { return run(add, add, erase, erase, apply, undo, answer, requested); }
    void reserve_queries(std::size_t capacity) { queries_.reserve(capacity); }
    void reserve_updates(std::size_t capacity) { updates_.reserve(capacity); }
    void clear_requests() noexcept { queries_.clear(); updates_.clear(); }
    void reset(std::int32_t n = 0) { MoWithUpdates next(n); swap(next); }
    void clear() noexcept { clear_requests(); length_ = 0; }
};
#endif
