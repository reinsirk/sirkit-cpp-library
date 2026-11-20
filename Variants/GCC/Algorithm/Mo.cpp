// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Algorithm/Mo.cpp
// 参考: https://nyaannyaan.github.io/library/misc/mo.hpp.html
// 区間移動とblock順序の数学的構成を参考に独立実装。更新版は時刻の往復、巻き戻し版は状態の保存と復元を加える。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/mo.md
// API一覧
// Mo(n=0): 長さn>=0の配列上の静的Mo。構築O(1)。add_query(l,r)は半開区間を記録し安定な要求IDを返す。
// MoWithUpdates(n=0): 構築O(1)。add_update(pos)が変更IDを返す。値と旧値は利用側が保持する。
//   add_query(l,r)は現在の変更数の時刻、add_query(l,r,time)は記録済みの任意時刻で照会を記録。
// QueryはMoでは{left,right}、更新版では{left,right,time}。query(id)は値、queries()はconst span。
// 更新版のupdate_position(id)/updates()は変更位置/そのconst span。追加は償却O(1)。確保失敗時内容維持。
// 共通: length()/query_count()/empty()は配列長/要求数/要求なし。更新版update_count()は変更数。
// block_size(b=0): 実際の幅B。正のbはその値、0は自動で1<=B<=max(1,n)。O(log(n+1))。
//   静的は整数演算でfloor(sqrt(2n^2/max(1,Q)))。更新版はQ=0を1として4QB^3>=3n^2B+12Un^2となる最小のBを倍精度で求める。
// order(b=0): runの処理順の要求ID vector。静的はleft block→right、更新版はleft/right block→timeで蛇行。
//   全sortキーが同じならID順。ID自体は追加順から変えない。O(Q log(Q+1)+log(n+1))時間・O(Q)作業領域。
// Mo.run(add_left,add_right,erase_left,erase_right,answer,b=0): 各伸縮でindex、answerで要求IDをstd::invoke。
//   run(add,erase,answer,b=0)は左右対称版。戻り値Window{left,right}は最後の区間、要求なしは{0,0}。
// Mo.run_rollback(add_left,add_right,snapshot,rollback,reset,answer,b=0): 削除を使わない巻き戻しMo。
//   run_rollback(add,snapshot,rollback,reset,answer,b=0)は左右対称版。幅はblock_size(b)。
//   各照会でsnapshot()→追加→answer→rollback()を呼ぶ。rollbackは直前のsnapshotの状態へ戻す。保存は同時に1つ。
//   resetは右側を伸ばしたblockの後に呼び、集計を空へ戻す。処理順はleft block→right昇順→ID。終了時は空。
// 更新版.run(add_left,add_right,erase_left,erase_right,apply,undo,answer,b=0):
//   apply/undo(change_id,left,right)で1時刻進退し、その後区間を移動する。run(add,erase,apply,undo,answer,b=0)も可。
//   戻り値State{left,right,time}。要求なしは{0,0,0}。
// 外部の初期集計は空、更新版の値は時刻0。最初の空区間は最初に処理する要求のleftに置く。
// run終了時の外部状態は戻り値の区間/時刻のまま。再実行前に利用側で初期状態へ戻す。
// callbackはlvalueとして呼べること。callback中に本体を変更/移動しない。例外時に本体記録/viewは維持するが外部状態は戻さない。
// 計算用領域はcallback前に確保する。N=n,Q=照会数,U=変更数,K=1+floor(N/B)として、sort以外の移動・追加回数は
//   静的・巻き戻しO(QB+NK+Q)、更新版O(QB+NK+UK^2+Q)。巻き戻しのresetは最大K回。callback自身の費用は別。
//   自動幅が最速とは保証しない。
// reserve_queries(cap)、更新版reserve_updates(cap): 記録容量を予約。clear_requests(): nを保ち全記録を消す。
// reset(n=0)/clear(): 全初期化/長さ0。copyはO(Q+U)で独立・代入強保証。move元は長さ0/記録なし、swap O(1)。
// 形・件数・要求/変更の取得はO(1)。reserveは再確保時O(Q)/O(U)。reset/clearは記録の破棄を含む。
// viewは追加/容量変更/clear/代入/所有権移動まで有効。番号/負数/区間/時刻/負の幅の検査はSKIP_RUNTIME_CHECKSで除外。
// 容量保護、幅0の自動選択、整数計算の0除算回避は検査省略時も維持。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MO_ORDERING_HELPERS_DEFINED
#define SIRKIT_MO_ORDERING_HELPERS_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
struct MoOrderEntry { std::size_t id; std::int32_t left, right; };
struct TimedMoOrderEntry { std::size_t id, time; std::int32_t left, right; };
inline std::uint64_t mo_integer_sqrt(std::uint64_t value) noexcept {
    std::uint64_t low = 0, high = std::min<std::uint64_t>(value, 0xFFFFFFFFu);
    while (low < high) {
        const auto distance = high - low, middle = low + distance / 2 + distance % 2;
        if (middle <= value / middle) low = middle; else high = middle - 1;
    }
    return low;
}
inline void mo_check_width(std::int32_t requested) {
#ifndef SKIP_RUNTIME_CHECKS
    if (requested < 0) throw std::invalid_argument("Mo block size");
#else
    (void)requested;
#endif
}
// 左端の往復QB/3と右端の掃引N^2/(2B)の見積りを基に、実測で係数2を選んだ。
inline std::int32_t mo_static_width(std::int32_t n, std::size_t queries, std::int32_t requested) {
    mo_check_width(requested);
    if (requested > 0) return requested;
    if (n <= 0) return 1;
    // int32_tの正の長さの二乗の2倍はuint64_tに収まる。
    const auto square = static_cast<std::uint64_t>(n) * static_cast<std::uint64_t>(n);
    const auto width = mo_integer_sqrt(2 * square / std::max<std::uint64_t>(1, queries));
    return static_cast<std::int32_t>(std::clamp<std::uint64_t>(width, 1, static_cast<std::uint64_t>(n)));
}
// 伸縮(2/3)QB+N^2/(2B)と時刻UK^2/2の見積りで、時刻1回を伸縮2回分として和を最小にする。
// 導関数が0以上になる最小のBを二分探索する。比較の丸め誤差は幅の選択だけに影響する。
inline std::int32_t mo_timed_width(std::int32_t n, std::size_t queries, std::size_t updates, std::int32_t requested) {
    mo_check_width(requested);
    if (requested > 0) return requested;
    if (n <= 0) return 1;
    const double count = static_cast<double>(std::max<std::size_t>(1, queries)), square = static_cast<double>(n) * static_cast<double>(n);
    const double time_cost = 12.0 * static_cast<double>(updates) * square;
    std::int32_t low = 1, high = n;
    while (low < high) {
        const auto middle = low + (high - low) / 2;
        const auto width = static_cast<double>(middle);
        if (4.0 * count * width * width * width >= 3.0 * square * width + time_cost) high = middle; else low = middle + 1;
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
// 下位id_bitsに追加順のIDを持つ相異なる鍵を昇順に並べる。
// 大きい入力は上位の比較鍵だけを安定な基数ソートで並べ、同じ比較鍵の要求はID順のまま残す。
inline void mo_sort_keys(std::vector<std::uint64_t>& keys, unsigned id_bits) {
    std::uint64_t used = 0;
    for (const auto key : keys) used |= key;
    const auto high = static_cast<unsigned>(std::bit_width(used));
    if (high <= id_bits) return;
    if (keys.size() < 1024) { std::sort(keys.begin(), keys.end()); return; }
    // 桁の種類数を要求数程度に抑え、各桁をO(Q)で処理する。
    const auto widest = std::clamp(static_cast<unsigned>(std::bit_width(keys.size())), 8u, 16u);
    const unsigned span = high - id_bits, passes = (span + widest - 1) / widest, digit = (span + passes - 1) / passes;
    const auto mask = (std::uint64_t{1} << digit) - 1;
    std::vector<std::uint64_t> buffer(keys.size());
    std::vector<std::size_t> start(std::size_t{1} << digit);
    for (unsigned pass = 0; pass < passes; ++pass) {
        const unsigned shift = id_bits + pass * digit;
        std::fill(start.begin(), start.end(), std::size_t{0});
        for (const auto key : keys) ++start[(key >> shift) & mask];
        if (start[(keys.front() >> shift) & mask] == keys.size()) continue;
        std::size_t sum = 0;
        for (auto& count : start) { const auto value = count; count = sum; sum += value; }
        for (const auto key : keys) buffer[start[(key >> shift) & mask]++] = key;
        keys.swap(buffer);
    }
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
    // left block→right→IDの順にemit(id)を呼ぶ。snakeなら奇数blockのrightを降順にする。
    template<class Emit>
    void sorted_ids(std::int32_t width, bool snake, Emit&& emit) const {
        const auto count = queries_.size();
        if (count == 0) return;
        const auto divisor = static_cast<std::uint32_t>(width);
        const auto block_bits = static_cast<unsigned>(std::bit_width(static_cast<std::uint32_t>(length_) / divisor));
        const auto right_bits = static_cast<unsigned>(std::bit_width(static_cast<std::uint32_t>(length_)));
        const auto id_bits = static_cast<unsigned>(std::bit_width(count - 1));
        if (block_bits + right_bits + id_bits <= 64) {
            const auto right_max = (std::uint64_t{1} << right_bits) - 1;
            std::vector<std::uint64_t> keys(count);
            for (std::size_t i = 0; i < count; ++i) {
                const auto block = static_cast<std::uint64_t>(static_cast<std::uint32_t>(queries_[i].left) / divisor);
                const auto right = static_cast<std::uint64_t>(queries_[i].right);
                keys[i] = (((block << right_bits) | (snake && (block & 1) ? right_max - right : right)) << id_bits) | i;
            }
            /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::mo_sort_keys(keys, id_bits);
            const auto id_mask = (std::uint64_t{1} << id_bits) - 1;
            for (const auto key : keys) emit(static_cast<std::size_t>(key & id_mask));
            return;
        }
        // 長さが大きく幅が小さいと鍵が64bitを超えるため、比較ソートで同じ順に並べる。
        std::vector<std::size_t> ids(count);
        for (std::size_t i = 0; i < count; ++i) ids[i] = i;
        std::sort(ids.begin(), ids.end(), [&](std::size_t a, std::size_t b) {
            const auto ab = queries_[a].left / width, bb = queries_[b].left / width;
            if (ab != bb) return ab < bb;
            const auto ar = queries_[a].right, br = queries_[b].right;
            if (ar != br) return snake && (ab & 1) ? ar > br : ar < br;
            return a < b;
        });
        for (const auto id : ids) emit(id);
    }
    std::vector</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::MoOrderEntry> ordered_entries(std::int32_t width, bool snake) const {
        std::vector</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::MoOrderEntry> result; result.reserve(queries_.size());
        sorted_ids(width, snake, [&](std::size_t id) { result.push_back({id, queries_[id].left, queries_[id].right}); });
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
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::mo_check_range(length_, left, right); const auto id = queries_.size(); queries_.push_back({left, right}); return id;
    }
    std::int32_t block_size(std::int32_t requested = 0) const { return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::mo_static_width(length_, queries_.size(), requested); }
    std::vector<std::size_t> order(std::int32_t requested = 0) const {
        const auto width = block_size(requested); std::vector<std::size_t> result; result.reserve(queries_.size());
        sorted_ids(width, true, [&](std::size_t id) { result.push_back(id); });
        return result;
    }
    template<class AddLeft, class AddRight, class EraseLeft, class EraseRight, class Answer>
    Window run(AddLeft&& add_left, AddRight&& add_right, EraseLeft&& erase_left, EraseRight&& erase_right, Answer&& answer, std::int32_t requested = 0) const {
        const auto entries = ordered_entries(block_size(requested), true); if (entries.empty()) return {0, 0};
        auto left = entries.front().left, right = left;
        // callbackの書き込みと別名にならないよう、要求は値で読む。
        for (const auto entry : entries) {
            while (left > entry.left) std::invoke(add_left, --left);
            while (right < entry.right) { std::invoke(add_right, right); ++right; }
            while (left < entry.left) { std::invoke(erase_left, left); ++left; }
            while (right > entry.right) std::invoke(erase_right, --right);
            std::invoke(answer, entry.id);
        }
        return {left, right};
    }
    template<class Add, class Erase, class Answer>
    Window run(Add&& add, Erase&& erase, Answer&& answer, std::int32_t requested = 0) const { return run(add, add, erase, erase, answer, requested); }
    template<class AddLeft, class AddRight, class Snapshot, class Rollback, class Reset, class Answer>
    requires std::is_invocable_v<Answer&, std::size_t>
    void run_rollback(AddLeft&& add_left, AddRight&& add_right, Snapshot&& snapshot, Rollback&& rollback, Reset&& reset, Answer&& answer, std::int32_t requested = 0) const {
        const auto width = block_size(requested); const auto entries = ordered_entries(width, false);
        for (std::size_t i = 0; i < entries.size();) {
            // 同じblockの要求はright昇順に並ぶ。block内に収まる要求が先に来るので、そのとき集計は空。
            const auto block = entries[i].left / width;
            const auto end = static_cast<std::int32_t>(std::min<std::int64_t>(length_, (std::int64_t{block} + 1) * width));
            auto right = end;
            for (; i < entries.size() && entries[i].left / width == block; ++i) {
                const auto entry = entries[i];
                if (entry.right <= end) {
                    std::invoke(snapshot);
                    for (auto index = entry.left; index < entry.right; ++index) std::invoke(add_right, index);
                } else {
                    while (right < entry.right) { std::invoke(add_right, right); ++right; }
                    std::invoke(snapshot);
                    for (auto index = end; index > entry.left;) std::invoke(add_left, --index);
                }
                std::invoke(answer, entry.id);
                std::invoke(rollback);
            }
            if (right > end) std::invoke(reset);
        }
    }
    template<class Add, class Snapshot, class Rollback, class Reset, class Answer>
    void run_rollback(Add&& add, Snapshot&& snapshot, Rollback&& rollback, Reset&& reset, Answer&& answer, std::int32_t requested = 0) const {
        run_rollback(add, add, snapshot, rollback, reset, answer, requested);
    }
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
    // left block→right block→timeの順にemit(id)を呼ぶ。right blockは奇数left blockで、timeは両blockの和が奇数で降順。
    template<class Emit>
    void sorted_ids(std::int32_t width, Emit&& emit) const {
        const auto count = queries_.size();
        if (count == 0) return;
        const auto divisor = static_cast<std::uint32_t>(width);
        const auto block_bits = static_cast<unsigned>(std::bit_width(static_cast<std::uint32_t>(length_) / divisor));
        const auto time_bits = static_cast<unsigned>(std::bit_width(updates_.size()));
        const auto id_bits = static_cast<unsigned>(std::bit_width(count - 1));
        if (2 * block_bits + time_bits + id_bits <= 64) {
            const auto block_max = (std::uint64_t{1} << block_bits) - 1, time_max = (std::uint64_t{1} << time_bits) - 1;
            std::vector<std::uint64_t> keys(count);
            for (std::size_t i = 0; i < count; ++i) {
                const auto& q = queries_[i];
                const auto left_block = static_cast<std::uint64_t>(static_cast<std::uint32_t>(q.left) / divisor);
                const auto right_block = static_cast<std::uint64_t>(static_cast<std::uint32_t>(q.right) / divisor);
                const auto time = static_cast<std::uint64_t>(q.time);
                const auto right_key = (left_block & 1) ? block_max - right_block : right_block;
                const auto time_key = ((left_block ^ right_block) & 1) ? time_max - time : time;
                keys[i] = (((((left_block << block_bits) | right_key) << time_bits) | time_key) << id_bits) | i;
            }
            /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::mo_sort_keys(keys, id_bits);
            const auto id_mask = (std::uint64_t{1} << id_bits) - 1;
            for (const auto key : keys) emit(static_cast<std::size_t>(key & id_mask));
            return;
        }
        // 長さが大きく幅が小さいと鍵が64bitを超えるため、比較ソートで同じ順に並べる。
        std::vector<std::size_t> ids(count);
        for (std::size_t i = 0; i < count; ++i) ids[i] = i;
        std::sort(ids.begin(), ids.end(), [&](std::size_t a, std::size_t b) {
            const auto& x = queries_[a]; const auto& y = queries_[b];
            const auto xl = x.left / width, yl = y.left / width, xr = x.right / width, yr = y.right / width;
            if (xl != yl) return xl < yl;
            if (xr != yr) return (xl & 1) ? xr > yr : xr < yr;
            if (x.time != y.time) return ((xl ^ xr) & 1) ? x.time > y.time : x.time < y.time;
            return a < b;
        });
        for (const auto id : ids) emit(id);
    }
    std::vector</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::TimedMoOrderEntry> ordered_entries(std::int32_t width) const {
        std::vector</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::TimedMoOrderEntry> result; result.reserve(queries_.size());
        sorted_ids(width, [&](std::size_t id) { const auto& q = queries_[id]; result.push_back({id, q.time, q.left, q.right}); });
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
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::mo_check_range(length_, left, right);
#ifndef SKIP_RUNTIME_CHECKS
        if (time > updates_.size()) throw std::out_of_range("Mo query time");
#endif
        const auto id = queries_.size(); queries_.push_back({left, right, time}); return id;
    }
    std::int32_t block_size(std::int32_t requested = 0) const { return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::mo_timed_width(length_, queries_.size(), updates_.size(), requested); }
    std::vector<std::size_t> order(std::int32_t requested = 0) const {
        const auto width = block_size(requested); std::vector<std::size_t> result; result.reserve(queries_.size());
        sorted_ids(width, [&](std::size_t id) { result.push_back(id); });
        return result;
    }
    template<class AddLeft, class AddRight, class EraseLeft, class EraseRight, class Apply, class Undo, class Answer>
    State run(AddLeft&& add_left, AddRight&& add_right, EraseLeft&& erase_left, EraseRight&& erase_right,
              Apply&& apply, Undo&& undo, Answer&& answer, std::int32_t requested = 0) const {
        const auto entries = ordered_entries(block_size(requested)); if (entries.empty()) return {0, 0, 0};
        auto left = entries.front().left, right = left; std::size_t time = 0;
        // callbackの書き込みと別名にならないよう、要求は値で読む。
        for (const auto entry : entries) {
            while (time < entry.time) { std::invoke(apply, time, left, right); ++time; }
            while (time > entry.time) std::invoke(undo, --time, left, right);
            while (left > entry.left) std::invoke(add_left, --left);
            while (right < entry.right) { std::invoke(add_right, right); ++right; }
            while (left < entry.left) { std::invoke(erase_left, left); ++left; }
            while (right > entry.right) std::invoke(erase_right, --right);
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

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
