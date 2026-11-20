#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/static_range_count_distinct

// Library: Algorithm/Mo.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Algorithm/Mo.cpp
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


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MO_ORDERING_HELPERS_DEFINED
#define SIRKIT_MO_ORDERING_HELPERS_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
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
    std::vector</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::MoOrderEntry> ordered_entries(std::int32_t requested) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (requested < 0) throw std::invalid_argument("Mo block size");
#endif
        std::vector</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::MoOrderEntry> result;
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
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::mo_check_range(length_, left, right); const auto id = queries_.size(); queries_.push_back({left, right}); return id;
    }
    std::int32_t block_size(std::int32_t requested = 0) const { return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::mo_block_width(length_, queries_.size(), requested, false); }
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
    std::vector</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::TimedMoOrderEntry> ordered_entries(std::int32_t requested) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (requested < 0) throw std::invalid_argument("Mo block size");
#endif
        std::vector</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::TimedMoOrderEntry> result;
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
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::mo_check_range(length_, left, right);
#ifndef SKIP_RUNTIME_CHECKS
        if (time > updates_.size()) throw std::out_of_range("Mo query time");
#endif
        const auto id = queries_.size(); queries_.push_back({left, right, time}); return id;
    }
    std::int32_t block_size(std::int32_t requested = 0) const { return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::mo_block_width(length_, queries_.size(), requested, true); }
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

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
// Verify I/O: Verify/current/common/fast_io.hpp
// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_VERIFY_FAST_IO_DEFINED
#define SIRKIT_VERIFY_FAST_IO_DEFINED

// VerifyInput(file=stdin): 非所有のFILEから読む。別の入力方式と混用しない。
// >> 整数: 64bit以下の十進整数を読む。検査省略時は値域内の正しい入力を前提とする。
// >> char/string: 空白を飛ばし、1文字／空白までの文字列を読む。
// >> 浮動小数点: 十進文字列を変換する。通常のCロケールで使う。
// bool / !: 直前までの読み取りが成功しているか／失敗したかを返す。
class VerifyInput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<unsigned char, buffer_size + 1> buffer_;
    std::size_t position_ = 0, size_ = 0;
    bool good_ = true;

    int peek() {
        if (position_ == size_) {
            size_ = std::fread(buffer_.data(), 1, buffer_size, file_);
            buffer_[size_] = 0; // 補充後の末尾は常に数字でない番兵。
            position_ = 0;
            if (size_ == 0) return EOF;
        }
        return buffer_[position_];
    }

    int skip_space() {
        int c;
        while ((c = peek()) != EOF && c <= ' ') ++position_;
        if (c == EOF) good_ = false;
        return c;
    }

public:
    explicit VerifyInput(std::FILE* file = stdin) : file_(file) {}
    VerifyInput(const VerifyInput&) = delete;
    VerifyInput& operator=(const VerifyInput&) = delete;

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyInput& operator>>(T& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        bool negative = false;
        if (c == '-' || c == '+') {
            negative = c == '-';
            ++position_;
            c = peek();
        }
        if (c == EOF) {
            good_ = false;
            return *this;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (c < '0' || c > '9') {
            good_ = false;
            return *this;
        }
        // 先頭の0を除けば、20桁以下の末尾直前の値はuint64_tに収まる。
        while (c == '0') {
            ++position_;
            c = peek();
        }
        std::uint64_t previous = 0;
        unsigned last_digit = 0;
        std::size_t digits = 0;
#endif
        std::uint64_t value = 0;
#ifdef SKIP_RUNTIME_CHECKS
        // バッファ末尾の番兵で内側の範囲判定を省き、数字が境界をまたぐときだけ補充する。
        for (;;) {
            const unsigned char* first = buffer_.data() + position_;
            const unsigned char* cursor = first;
            while (static_cast<unsigned>(*cursor - '0') < 10) {
                value = value * 10 + static_cast<unsigned>(*cursor - '0');
                ++cursor;
            }
            position_ += static_cast<std::size_t>(cursor - first);
            if (position_ != size_) break;
            c = peek();
            if (c == EOF) break;
        }
#else
        while (c >= '0' && c <= '9') {
            previous = value;
            last_digit = static_cast<unsigned>(c - '0');
            if (digits <= 20) ++digits;
            value = value * 10 + static_cast<unsigned>(c - '0');
            ++position_;
            c = peek();
        }
#endif
#ifndef SKIP_RUNTIME_CHECKS
        constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
        if ((c != EOF && c > ' ') || digits > 20
            || (digits == 20 && (previous > maximum / 10
                || (previous == maximum / 10 && last_digit > maximum % 10)))) {
            good_ = false;
            return *this;
        }
        std::uint64_t limit = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
        if constexpr (std::is_signed_v<T>) limit += negative;
        else if (negative && value != 0) good_ = false;
        if (value > limit) good_ = false;
        if (!good_) return *this;
#endif
        if constexpr (std::is_signed_v<T>) {
            constexpr std::uint64_t minimum_magnitude =
                static_cast<std::uint64_t>(std::numeric_limits<T>::max()) + 1;
            // -(最小値)は評価せず、絶対値が最大値+1のときだけ最小値を使う。
            result = negative
                ? (value == minimum_magnitude ? std::numeric_limits<T>::min()
                                  : static_cast<T>(-static_cast<T>(value)))
                : static_cast<T>(value);
        } else result = static_cast<T>(value);
        return *this;
    }

    VerifyInput& operator>>(char& result) {
        if (!good_) return *this;
        const int c = skip_space();
        if (c != EOF) {
            result = static_cast<char>(c);
            ++position_;
        }
        return *this;
    }

    VerifyInput& operator>>(std::string& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        result.clear();
        for (;;) {
            const std::size_t first = position_;
            while (position_ < size_ && buffer_[position_] > ' ') ++position_;
            result.append(reinterpret_cast<const char*>(buffer_.data() + first), position_ - first);
            if (position_ != size_ || peek() == EOF) break;
        }
        return *this;
    }

    template<class T> requires std::is_floating_point_v<T>
    VerifyInput& operator>>(T& result) {
        std::string token;
        *this >> token;
        if (!good_) return *this;
#ifndef SKIP_RUNTIME_CHECKS
        char* end = nullptr;
        const long double value = std::strtold(token.c_str(), &end);
        if (end != token.c_str() + token.size()) good_ = false;
        else result = static_cast<T>(value);
#else
        result = static_cast<T>(std::strtold(token.c_str(), nullptr));
#endif
        return *this;
    }
};

// VerifyOutput(file=stdout): 非所有のFILEへ書く。破棄時にflushし、FILEは閉じない。
// << 整数/char/文字列/浮動小数点: 値を出力する。整数は64bit以下に対応する。
// fixed(digits): 浮動小数点を小数点以下digits桁で出力する（digits>=0）。
// flush(): バッファの内容をFILEへ渡す。bool / !で書き込み失敗を確認できる。
class VerifyOutput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<char, buffer_size> buffer_;
    std::size_t size_ = 0;
    int precision_ = 6;
    bool fixed_ = false, good_ = true;

    void write(std::string_view text) {
        while (!text.empty() && good_) {
            if (size_ == buffer_.size()) flush();
            const auto count = std::min(text.size(), buffer_.size() - size_);
            std::memcpy(buffer_.data() + size_, text.data(), count);
            size_ += count;
            text.remove_prefix(count);
        }
    }

public:
    explicit VerifyOutput(std::FILE* file = stdout) : file_(file) {}
    VerifyOutput(const VerifyOutput&) = delete;
    VerifyOutput& operator=(const VerifyOutput&) = delete;
    ~VerifyOutput() { flush(); }

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    VerifyOutput& fixed(int digits) {
        if (digits < 0) throw std::invalid_argument("negative output precision");
        fixed_ = true;
        precision_ = digits;
        return *this;
    }

    void flush() noexcept {
        std::size_t written = 0;
        while (written < size_ && good_) {
            const auto count = std::fwrite(buffer_.data() + written, 1, size_ - written, file_);
            if (count == 0) good_ = false;
            written += count;
        }
        size_ = 0;
    }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyOutput& operator<<(T value) {
        if constexpr (std::is_same_v<T, bool>) return *this << static_cast<char>('0' + value);
        else {
            constexpr std::size_t capacity = std::numeric_limits<std::uint64_t>::digits10 + 3;
            if (buffer_.size() - size_ < capacity) flush();
            if (!good_) return *this;
            const auto end = std::to_chars(buffer_.data() + size_, buffer_.data() + size_ + capacity, value).ptr;
            size_ = static_cast<std::size_t>(end - buffer_.data());
            return *this;
        }
    }

    VerifyOutput& operator<<(char value) {
        if (size_ == buffer_.size()) flush();
        if (good_) buffer_[size_++] = value;
        return *this;
    }

    VerifyOutput& operator<<(std::string_view value) { write(value); return *this; }
    VerifyOutput& operator<<(const std::string& value) { write(value); return *this; }
    VerifyOutput& operator<<(const char* value) { write(value); return *this; }

    template<class T> requires std::is_floating_point_v<T>
    VerifyOutput& operator<<(T value) {
        char local[128];
        const char* format = fixed_ ? "%.*Lf" : "%.*Lg";
        const int count = std::snprintf(local, sizeof(local), format, precision_,
                                        static_cast<long double>(value));
        if (count < 0) good_ = false;
        else if (static_cast<std::size_t>(count) < sizeof(local))
            write(std::string_view(local, static_cast<std::size_t>(count)));
        else {
            std::vector<char> text(static_cast<std::size_t>(count) + 1);
            std::snprintf(text.data(), text.size(), format, precision_, static_cast<long double>(value));
            write(std::string_view(text.data(), static_cast<std::size_t>(count)));
        }
        return *this;
    }
};

inline VerifyInput verify_in;
inline VerifyOutput verify_out;
#endif

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
int main(){
    std::int32_t n{},q{};verify_in>>n>>q;
    std::vector<std::int32_t>values(static_cast<std::size_t>(n));for(auto&v:values)verify_in>>v;
    auto keys=values;std::sort(keys.begin(),keys.end());keys.erase(std::unique(keys.begin(),keys.end()),keys.end());
    for(auto&v:values)v=static_cast<std::int32_t>(std::lower_bound(keys.begin(),keys.end(),v)-keys.begin());
    Mo mo(n);
    for(std::int32_t i=0;i<q;++i){std::int32_t l{},r{};verify_in>>l>>r;mo.add_query(l,r);}
    std::vector<std::int32_t>frequency(keys.size()),answers(mo.query_count());std::int32_t distinct=0;
    auto add=[&](std::int32_t i){if(frequency[static_cast<std::size_t>(values[static_cast<std::size_t>(i)])]++==0)++distinct;};
    auto erase=[&](std::int32_t i){if(--frequency[static_cast<std::size_t>(values[static_cast<std::size_t>(i)])]==0)--distinct;};
    mo.run(add,erase,[&](std::size_t id){answers[id]=distinct;});
    for(const auto answer:answers)verify_out<<answer<<'\n';
}
