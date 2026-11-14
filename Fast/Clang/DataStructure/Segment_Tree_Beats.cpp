// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Segment_Tree_Beats.cpp
// 参考（構成・問題選定）: https://nyaannyaan.github.io/library/segment-tree/segment-tree-beats.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/segment_tree_beats.md
//
// API一覧
// [SegmentTreeBeats]
// SegmentTreeBeats()/SegmentTreeBeats(vector<int64_t>): 空/列から構築。O(n+1)時間・領域。
// reset(vector<int64_t>): 再構築。O(旧n+n+1)。確保失敗時は元の列を保つ。
// chmin(l,r,x)/chmax(l,r,x)/add(l,r,x): 半開区間[l,r)をmin/max/加算で更新。添字は0始まり。
// 更新列全体の償却計算量はO((n+q) log²(n+1))、1回は最悪O(n+1)。set(i,x): 点代入、O(log(n+1))。
// sum(l,r): 区間和、空なら0。min(l,r)/max(l,r): optional<int64_t>、空ならnullopt。O(log(n+1))。
// get(i): 値、O(log(n+1))。values(): 全要素をコピー、O(n+1)。取得操作も遅延を伝搬するため非const。
// size()/empty(): 要素数/空か、O(1)。clear(): 空へ戻す、O(n+1)。swap(other): O(1)。コピーは独立、移動元は空。
// 値はint64_t全域。add結果の要素が範囲外ならoverflow_error（変更なし）。sum結果が範囲外でもoverflow_error。
// 内部和/遅延はGCC/Clangの128bit拡張、SIRKIT_FORCE_STANDARD_ARITHMETIC時は標準C++の2ワード演算。
// 桁あふれ・容量検査はSKIP_RUNTIME_CHECKS時も維持。添字/区間検査だけを同マクロで除く。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SEGMENT_TREE_BEATS_WIDE_DEFINED
#define SIRKIT_SEGMENT_TREE_BEATS_WIDE_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
using BeatsWide = __int128_t;
inline BeatsWide beats_multiply(BeatsWide value, std::size_t count) noexcept { return value * static_cast<BeatsWide>(count); }
inline std::int64_t beats_narrow(BeatsWide value) {
    if (value < std::numeric_limits<std::int64_t>::min() || value > std::numeric_limits<std::int64_t>::max())
        throw std::overflow_error("SegmentTreeBeats: int64 overflow");
    return static_cast<std::int64_t>(value);
}
#else
// 符号と128bitの絶対値。使用範囲は2^127未満。
class BeatsWide;
inline BeatsWide beats_multiply(BeatsWide, std::size_t) noexcept;
inline std::int64_t beats_narrow(BeatsWide);
class BeatsWide {
    std::uint64_t low_ = 0, high_ = 0;
    bool negative_ = false;
    void normalize() noexcept { if ((low_ | high_) == 0) negative_ = false; }
public:
    BeatsWide() = default;
    BeatsWide(std::int64_t value) noexcept
        : low_(value < 0 ? std::uint64_t{0} - static_cast<std::uint64_t>(value) : static_cast<std::uint64_t>(value)), negative_(value < 0) {}
    friend bool operator==(const BeatsWide&, const BeatsWide&) = default;
    friend bool operator<(const BeatsWide& a, const BeatsWide& b) noexcept {
        if (a.negative_ != b.negative_) return a.negative_;
        if (a.high_ != b.high_) return a.negative_ ? a.high_ > b.high_ : a.high_ < b.high_;
        return a.negative_ ? a.low_ > b.low_ : a.low_ < b.low_;
    }
    friend bool operator>(const BeatsWide& a, const BeatsWide& b) noexcept { return b < a; }
    friend bool operator<=(const BeatsWide& a, const BeatsWide& b) noexcept { return !(b < a); }
    friend bool operator>=(const BeatsWide& a, const BeatsWide& b) noexcept { return !(a < b); }
    BeatsWide operator-() const noexcept {
        auto result = *this;
        if ((low_ | high_) != 0) result.negative_ = !negative_;
        return result;
    }
    friend BeatsWide operator+(BeatsWide a, const BeatsWide& b) noexcept {
        if (a.negative_ == b.negative_) {
            const auto old = a.low_;
            a.low_ += b.low_;
            a.high_ += b.high_ + static_cast<std::uint64_t>(a.low_ < old);
        } else {
            const bool smaller = a.high_ < b.high_ || (a.high_ == b.high_ && a.low_ < b.low_);
            if (smaller) {
                const auto borrow = static_cast<std::uint64_t>(b.low_ < a.low_);
                a.low_ = b.low_ - a.low_;
                a.high_ = b.high_ - a.high_ - borrow;
                a.negative_ = b.negative_;
            } else {
                const auto borrow = static_cast<std::uint64_t>(a.low_ < b.low_);
                a.low_ -= b.low_;
                a.high_ -= b.high_ + borrow;
            }
        }
        a.normalize(); return a;
    }
    friend BeatsWide operator-(BeatsWide a, const BeatsWide& b) noexcept { return a + -b; }
    BeatsWide& operator+=(const BeatsWide& b) noexcept { return *this = *this + b; }
    friend BeatsWide beats_multiply(BeatsWide a, std::size_t count) noexcept {
        static_assert(sizeof(std::size_t) <= sizeof(std::uint64_t));
        const auto b = static_cast<std::uint64_t>(count);
        constexpr std::uint64_t mask = 0xffffffff;
        const auto a0 = a.low_ & mask, a1 = a.low_ >> 32;
        const auto b0 = b & mask, b1 = b >> 32;
        const auto p00 = a0 * b0;
        const auto middle = a1 * b0 + (p00 >> 32);
        const auto next = a0 * b1 + (middle & mask);
        a.high_ = a.high_ * b + a1 * b1 + (middle >> 32) + (next >> 32);
        a.low_ = (next << 32) | (p00 & mask);
        a.normalize(); return a;
    }
    friend std::int64_t beats_narrow(BeatsWide value) {
        constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
        const auto bound = static_cast<std::uint64_t>(maximum) + static_cast<unsigned>(value.negative_);
        if (value.high_ != 0 || value.low_ > bound)
            throw std::overflow_error("SegmentTreeBeats: int64 overflow");
        if (value.negative_ && value.low_ == bound) return std::numeric_limits<std::int64_t>::min();
        const auto magnitude = static_cast<std::int64_t>(value.low_);
        return value.negative_ ? -magnitude : magnitude;
    }
};
#endif
} // namespace reinsirk::detail
#endif

#ifndef SIRKIT_SEGMENT_TREE_BEATS_DEFINED
#define SIRKIT_SEGMENT_TREE_BEATS_DEFINED
class SegmentTreeBeats {
    using Wide = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::BeatsWide;
    struct Node {
        Wide sum{}, add{};
        std::int64_t minimum = 0, maximum = 0, second_minimum = 0, second_maximum = 0;
        std::size_t minimum_count = 0, maximum_count = 0;
    };
    std::size_t size_ = 0;
    std::vector<Node> nodes_;
    static Wide multiply(Wide value, std::size_t count) noexcept {
        using /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::beats_multiply;
        return beats_multiply(value, count);
    }
    static std::int64_t narrow(Wide value) {
        using /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::beats_narrow;
        return beats_narrow(value);
    }
    static void uniform(Node& node, std::int64_t value, std::size_t length) noexcept {
        node.sum = multiply(Wide(value), length); node.add = Wide(0);
        node.minimum = node.maximum = value;
        node.minimum_count = node.maximum_count = length;
        node.second_minimum = node.second_maximum = value;
    }
    void check_range(std::size_t l, std::size_t r) const {
        (void)l; (void)r;
    }
    void check_index(std::size_t i) const {
        (void)i;
    }
    void pull(std::size_t p, std::size_t length) noexcept {
        const auto& a = nodes_[p * 2]; const auto& b = nodes_[p * 2 + 1];
        auto& c = nodes_[p]; c.sum = a.sum + b.sum; c.add = Wide(0);
        c.minimum = std::min(a.minimum, b.minimum);
        c.maximum = std::max(a.maximum, b.maximum);
        c.minimum_count = (a.minimum == c.minimum ? a.minimum_count : 0) + (b.minimum == c.minimum ? b.minimum_count : 0);
        c.maximum_count = (a.maximum == c.maximum ? a.maximum_count : 0) + (b.maximum == c.maximum ? b.maximum_count : 0);
        // 二番目が存在するかは個数で判定し、値の番兵を使わない。
        if (c.minimum_count != length) {
            auto second = c.maximum;
            if (a.minimum != c.minimum) second = std::min(second, a.minimum);
            else if (a.minimum != a.maximum) second = std::min(second, a.second_minimum);
            if (b.minimum != c.minimum) second = std::min(second, b.minimum);
            else if (b.minimum != b.maximum) second = std::min(second, b.second_minimum);
            c.second_minimum = second;
        }
        if (c.maximum_count != length) {
            auto second = c.minimum;
            if (a.maximum != c.maximum) second = std::max(second, a.maximum);
            else if (a.minimum != a.maximum) second = std::max(second, a.second_maximum);
            if (b.maximum != c.maximum) second = std::max(second, b.maximum);
            else if (b.minimum != b.maximum) second = std::max(second, b.second_maximum);
            c.second_maximum = second;
        }
    }
    void build(std::size_t p, std::size_t l, std::size_t r, const std::vector<std::int64_t>& values) noexcept {
        if (r - l == 1) { uniform(nodes_[p], values[l], 1); return; }
        const auto m = l + (r - l) / 2;
        build(p * 2, l, m, values); build(p * 2 + 1, m, r, values); pull(p, r - l);
    }
    void cap(std::size_t p, std::size_t length, std::int64_t value) noexcept {
        auto& node = nodes_[p];
        if (value >= node.maximum) return;
        if (value <= node.minimum) { uniform(node, value, length); return; }
        node.sum += multiply(Wide(value) - Wide(node.maximum), node.maximum_count);
        if (node.second_minimum == node.maximum) node.second_minimum = value;
        node.maximum = value;
    }
    void floor(std::size_t p, std::size_t length, std::int64_t value) noexcept {
        auto& node = nodes_[p];
        if (value <= node.minimum) return;
        if (value >= node.maximum) { uniform(node, value, length); return; }
        node.sum += multiply(Wide(value) - Wide(node.minimum), node.minimum_count);
        if (node.second_maximum == node.minimum) node.second_maximum = value;
        node.minimum = value;
    }
    // 子の古い値+addが一時的にint64を超えても、clamp完了後にだけ狭める。
    void transform(std::size_t p, std::size_t length, Wide add, std::int64_t lower, std::int64_t upper) {
        auto& node = nodes_[p];
        if (add == Wide(0) && lower <= node.minimum && node.maximum <= upper) return;
        auto lo = Wide(node.minimum) + add, hi = Wide(node.maximum) + add;
        if (lo >= Wide(upper)) { uniform(node, upper, length); return; }
        if (hi <= Wide(lower)) { uniform(node, lower, length); return; }
        if (node.minimum == node.maximum) {
            uniform(node, narrow(lo), length); return;
        }
        auto second_lo = Wide(node.second_minimum) + add;
        auto second_hi = Wide(node.second_maximum) + add;
        node.sum += multiply(add, length); node.add += add;
        if (hi > Wide(upper)) {
            node.sum += multiply(Wide(upper) - hi, node.maximum_count);
            if (second_lo == hi) second_lo = Wide(upper);
            hi = Wide(upper);
        }
        if (lo < Wide(lower)) {
            node.sum += multiply(Wide(lower) - lo, node.minimum_count);
            if (second_hi == lo) second_hi = Wide(lower);
            lo = Wide(lower);
        }
        node.minimum = narrow(lo); node.maximum = narrow(hi);
        node.second_minimum = narrow(second_lo); node.second_maximum = narrow(second_hi);
    }
    void push(std::size_t p, std::size_t l, std::size_t r) {
        if (r - l == 1) return;
        const auto m = l + (r - l) / 2;
        const auto add = nodes_[p].add;
        const auto low = nodes_[p].minimum, high = nodes_[p].maximum;
        if (low == high) {
            uniform(nodes_[p * 2], low, m - l); uniform(nodes_[p * 2 + 1], low, r - m);
        } else {
            transform(p * 2, m - l, add, low, high);
            transform(p * 2 + 1, r - m, add, low, high);
        }
        nodes_[p].add = Wide(0);
    }
    void update(std::size_t p, std::size_t l, std::size_t r, std::size_t ql, std::size_t qr, std::int64_t x, unsigned operation) {
        if (r <= ql || qr <= l) return;
        const auto& node = nodes_[p];
        if (operation == 0 && node.maximum <= x) return;
        if (operation == 1 && node.minimum >= x) return;
        if (ql <= l && r <= qr) {
            if (operation == 0 && (x <= node.minimum || node.second_maximum < x)) { cap(p, r - l, x); return; }
            if (operation == 1 && (node.maximum <= x || x < node.second_minimum)) { floor(p, r - l, x); return; }
            if (operation == 2) {
                transform(p, r - l, Wide(x), std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max()); return;
            }
        }
        push(p, l, r);
        const auto m = l + (r - l) / 2;
        update(p * 2, l, m, ql, qr, x, operation);
        update(p * 2 + 1, m, r, ql, qr, x, operation);
        pull(p, r - l);
    }
    Wide query_sum(std::size_t p, std::size_t l, std::size_t r, std::size_t ql, std::size_t qr) {
        if (r <= ql || qr <= l) return Wide(0);
        if (ql <= l && r <= qr) return nodes_[p].sum;
        push(p, l, r); const auto m = l + (r - l) / 2;
        return query_sum(p * 2, l, m, ql, qr) + query_sum(p * 2 + 1, m, r, ql, qr);
    }
    std::int64_t query_extreme(std::size_t p, std::size_t l, std::size_t r, std::size_t ql, std::size_t qr, bool maximum) {
        if (ql <= l && r <= qr) return maximum ? nodes_[p].maximum : nodes_[p].minimum;
        push(p, l, r); const auto m = l + (r - l) / 2;
        if (qr <= m) return query_extreme(p * 2, l, m, ql, qr, maximum);
        if (m <= ql) return query_extreme(p * 2 + 1, m, r, ql, qr, maximum);
        const auto a = query_extreme(p * 2, l, m, ql, qr, maximum);
        const auto b = query_extreme(p * 2 + 1, m, r, ql, qr, maximum);
        return maximum ? std::max(a, b) : std::min(a, b);
    }
    void assign_point(std::size_t p, std::size_t l, std::size_t r, std::size_t i, std::int64_t value) {
        if (r - l == 1) { uniform(nodes_[p], value, 1); return; }
        push(p, l, r); const auto m = l + (r - l) / 2;
        if (i < m) assign_point(p * 2, l, m, i, value);
        else assign_point(p * 2 + 1, m, r, i, value);
        pull(p, r - l);
    }
    void collect(std::size_t p, std::size_t l, std::size_t r, std::vector<std::int64_t>& result) {
        if (r - l == 1) { result[l] = nodes_[p].minimum; return; }
        push(p, l, r); const auto m = l + (r - l) / 2;
        collect(p * 2, l, m, result); collect(p * 2 + 1, m, r, result);
    }
public:
    SegmentTreeBeats() = default;
    explicit SegmentTreeBeats(const std::vector<std::int64_t>& values) : size_(values.size()) {
        if (size_ == 0) return;
        if (size_ > (nodes_.max_size() - 1) / 4 || size_ > std::numeric_limits<std::int64_t>::max())
            throw std::length_error("SegmentTreeBeats: capacity");
        nodes_.resize(size_ * 4 + 1); build(1, 0, size_, values);
    }
    SegmentTreeBeats(const SegmentTreeBeats&) = default;
    SegmentTreeBeats& operator=(const SegmentTreeBeats& other) {
        if (this != &other) { SegmentTreeBeats replacement(other); swap(replacement); }
        return *this;
    }
    SegmentTreeBeats(SegmentTreeBeats&& other) noexcept : size_(std::exchange(other.size_, 0)), nodes_(std::move(other.nodes_)) {}
    SegmentTreeBeats& operator=(SegmentTreeBeats&& other) noexcept {
        if (this != &other) { SegmentTreeBeats replacement(std::move(other)); swap(replacement); }
        return *this;
    }
    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    void reset(const std::vector<std::int64_t>& values) { SegmentTreeBeats replacement(values); swap(replacement); }
    void clear() noexcept { nodes_.clear(); size_ = 0; }
    void swap(SegmentTreeBeats& other) noexcept { std::swap(size_, other.size_); nodes_.swap(other.nodes_); }
    void chmin(std::size_t l, std::size_t r, std::int64_t x) { check_range(l, r); if (l != r) update(1, 0, size_, l, r, x, 0); }
    void chmax(std::size_t l, std::size_t r, std::int64_t x) { check_range(l, r); if (l != r) update(1, 0, size_, l, r, x, 1); }
    void add(std::size_t l, std::size_t r, std::int64_t x) {
        check_range(l, r); if (l == r || x == 0) return;
        // 実際の値だけを検査し、子に残る古い値は検査しない。
        if (x > 0) (void)narrow(Wide(query_extreme(1, 0, size_, l, r, true)) + Wide(x));
        else (void)narrow(Wide(query_extreme(1, 0, size_, l, r, false)) + Wide(x));
        update(1, 0, size_, l, r, x, 2);
    }
    void set(std::size_t i, std::int64_t value) { check_index(i); assign_point(1, 0, size_, i, value); }
    std::int64_t get(std::size_t i) { check_index(i); return query_extreme(1, 0, size_, i, i + 1, false); }
    std::int64_t sum(std::size_t l, std::size_t r) { check_range(l, r); return l == r ? 0 : narrow(query_sum(1, 0, size_, l, r)); }
    std::optional<std::int64_t> min(std::size_t l, std::size_t r) {
        check_range(l, r); if (l == r) return std::nullopt;
        return query_extreme(1, 0, size_, l, r, false);
    }
    std::optional<std::int64_t> max(std::size_t l, std::size_t r) {
        check_range(l, r); if (l == r) return std::nullopt;
        return query_extreme(1, 0, size_, l, r, true);
    }
    std::vector<std::int64_t> values() {
        std::vector<std::int64_t> result(size_);
        if (size_ != 0) collect(1, 0, size_, result);
        return result;
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
