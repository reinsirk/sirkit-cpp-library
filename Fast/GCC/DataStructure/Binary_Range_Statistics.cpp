// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Binary_Range_Statistics.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/binary_range_statistics.md
//
// API一覧
// BinaryRangeStatistics(bits=""): '0'と'1'の列から構築。BinaryRangeStatistics(n): 0をn個。O(n)時間・領域。
// flip(l,r): [l,r)の0と1を反転。assign(l,r,bit): 区間を指定ビットにする。O(log(n+1))。
// stats(l,r): length,onesと、0/1それぞれのprefix/suffix/longestを返す。空区間は各項目0。O(log(n+1))。
// all_stats(): 全体統計、size(): 要素数。O(1)。添字は0始まり、bitはbool。コピーは独立。
//
// [同梱の遅延セグメント木]
// [lazy_segtree<S,op,e,F,mapping,composition,id>]
// lazy_segtree()/lazy_segtree(int n)/lazy_segtree(vector<S>): 空列/e()のn要素/列から構築。O(n+1)時間・領域。
// op(S,S)は結合的でe()が左右の単位元。mapping(F,S)は作用、composition(newer,older)はolderの後にnewerを適用し、id()は恒等作用。
// mapping(F,S,size_t length)も拡張として使用可。両方呼べる場合は2引数を使う。演算・コピーはO(1)とする。
// 添字・区間端・nはint、0<=n<=INT_MAXで、内部容量にも収まること。区間は0始まりの半開。size()/empty(): 要素数/空か、O(1)。
// 点添字は0<=p<size()、区間は0<=l<=r<=size()。
// set(int p,S x)/apply(int p,F f)/apply(int l,int r,F f): 点代入/点作用/区間作用。get(int p)/prod(int l,int r): 点の値/左からの区間積。O(log(n+1))。
// all_prod(): 全積のS値、O(1)。空区間・空列の積はe()。照会でも遅延伝播で内部状態を変更する。
// max_right<pred>(int l)/min_left<pred>(int r): 関数bool pred(S)で境界探索。max_right(l,pred)/min_left(r,pred): 関数オブジェクト版。戻り値int、O(log(n+1))。
// pred(e())==true、同じ値に同じ結果を返し、区間を延ばすと真から偽へ一度だけ変わること。単調性のために全要素を走査しない。
// values(): 全点を伝播して列を返す。O(n+1)。assign(vector<S>)/reset(int n)/reset(): 列を再構築/n要素/現在の長さを単位元へ戻す。O(旧n+n+1)。
// clear()/swap(other): 空へ戻す/交換。O(旧n+1)/O(1)。コピーは独立、移動元は空。演算等が例外を投げる場合は内容の強い保証なし。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include <array>
#include <string_view>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SEGMENT_ARRAY_CAPACITY_DEFINED
#define SIRKIT_SEGMENT_ARRAY_CAPACITY_DEFINED
namespace detail {
inline std::size_t CheckedSegmentLeafBase(std::size_t count, std::size_t limit, const char* error) {
    if (count == 0) return 1;
    if (count > std::bit_floor(limit)) throw std::length_error(error);
    return std::bit_ceil(count);
}

inline int CheckedSegmentArrayCount(std::size_t count, const char* error) {
    if (count > static_cast<std::size_t>(std::numeric_limits<int>::max())) throw std::length_error(error);
    return static_cast<int>(count);
}
}
#endif

#ifndef SIRKIT_LAZY_SEGMENT_TREE_DEFINED
#define SIRKIT_LAZY_SEGMENT_TREE_DEFINED
template<class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
class lazy_segtree {
    static_assert(std::is_invocable_r_v<S, decltype(op), const S&, const S&>);
    static_assert(std::is_invocable_r_v<S, decltype(e)>);
    static_assert(std::is_invocable_r_v<S, decltype(mapping), const F&, const S&>
        || std::is_invocable_r_v<S, decltype(mapping), const F&, const S&, std::size_t>);
    static_assert(std::is_invocable_r_v<F, decltype(composition), const F&, const F&>);
    static_assert(std::is_invocable_r_v<F, decltype(id)>);
    S value_identity_;
    F update_identity_;
    std::size_t n_ = 0, base_ = 1, height_ = 0;
    std::vector<S> data_;
    std::vector<F> lazy_;
    std::vector<std::uint8_t> pending_;
    static std::size_t leaf_base(std::size_t n) {
        const auto limit = std::min({std::vector<S>{}.max_size() / 2,
                                     std::vector<F>{}.max_size(), std::vector<std::uint8_t>{}.max_size()});
        return detail::CheckedSegmentLeafBase(n, limit, "lazy_segtree: size");
    }
    static std::size_t checked_size(int n) {
        return static_cast<std::size_t>(n);
    }
    static int vector_size(const std::vector<S>& values) {
        return detail::CheckedSegmentArrayCount(values.size(), "lazy_segtree: size");
    }
    void check_index(int i) const {
        (void)i;
    }
    void check_range(int l, int r) const {
        (void)l; (void)r;
    }
    static std::size_t aligned_node(std::size_t value) {
        if (value == 0) std::unreachable();
        return value >> std::countr_zero(value);
    }
    void pull(std::size_t p) { data_[p] = std::invoke(op, data_[2 * p], data_[2 * p + 1]); }
    void act(std::size_t p, const F& f, std::size_t length) {
        if constexpr (std::is_invocable_r_v<S, decltype(mapping), const F&, const S&>)
            data_[p] = std::invoke(mapping, f, data_[p]);
        else data_[p] = std::invoke(mapping, f, data_[p], length);
        if (p < base_) {
            if (pending_[p]) lazy_[p] = std::invoke(composition, f, lazy_[p]);
            else { lazy_[p] = f; pending_[p] = 1; }
        }
    }
    void push(std::size_t p, std::size_t length) {
        if (!pending_[p]) return;
        act(2 * p, lazy_[p], length / 2);
        act(2 * p + 1, lazy_[p], length / 2);
        pending_[p] = 0;
    }
    void push_path(std::size_t leaf) {
        for (auto level = height_; level; --level) push(leaf >> level, std::size_t{1} << level);
    }
    void push_boundary(std::size_t l, std::size_t r) {
        for (auto level = height_; level; --level) {
            const auto length = std::size_t{1} << level;
            if (l & (length - 1)) push(l >> level, length);
            if (r & (length - 1)) push((r - 1) >> level, length);
        }
    }
    void pull_path(std::size_t leaf) {
        for (leaf >>= 1; leaf; leaf >>= 1) pull(leaf);
    }
public:
    lazy_segtree() : lazy_segtree(0) {}
    explicit lazy_segtree(int n)
        : value_identity_(std::invoke(e)), update_identity_(std::invoke(id)), n_(checked_size(n)), base_(leaf_base(n_)),
          data_(n_ ? 2 * base_ : 0, value_identity_), lazy_(n_ ? base_ : 0, update_identity_), pending_(n_ ? base_ : 0, 0) {
        for (auto p = base_; p > 1; p >>= 1) ++height_;
    }
    explicit lazy_segtree(const std::vector<S>& values) : lazy_segtree(vector_size(values)) {
        if (!n_) return;
        std::copy(values.begin(), values.end(), data_.begin() + base_);
        for (auto p = base_; p > 1;) pull(--p);
    }
    lazy_segtree(const lazy_segtree&) = default;
    lazy_segtree& operator=(const lazy_segtree& other) {
        if (this != &other) { lazy_segtree replacement(other); swap(replacement); }
        return *this;
    }
    lazy_segtree(lazy_segtree&& other)
        : value_identity_(other.value_identity_), update_identity_(other.update_identity_),
          n_(other.n_), base_(other.base_), height_(other.height_), data_(std::move(other.data_)),
          lazy_(std::move(other.lazy_)), pending_(std::move(other.pending_)) {
        other.clear();
    }
    lazy_segtree& operator=(lazy_segtree&& other) {
        if (this != &other) { lazy_segtree replacement(std::move(other)); swap(replacement); }
        return *this;
    }
    int size() const noexcept { return static_cast<int>(n_); }
    bool empty() const noexcept { return n_ == 0; }
    S get(int i) {
        check_index(i);
        const auto leaf = base_ + i;
        push_path(leaf);
        return data_[leaf];
    }
    void set(int i, S value) {
        check_index(i);
        const auto leaf = base_ + i;
        push_path(leaf); data_[leaf] = value; pull_path(leaf);
    }
    void apply(int i, F f) {
        check_index(i);
        const auto leaf = base_ + i;
        push_path(leaf); act(leaf, f, 1); pull_path(leaf);
    }
    void apply(int l, int r, F f) {
        check_range(l, r);
        if (l == r) return;
        const auto begin = base_ + static_cast<std::size_t>(l);
        const auto end = base_ + static_cast<std::size_t>(r);
        push_boundary(begin, end);
        auto left = begin, right = end;
        std::size_t length = 1;
        while (left < right) {
            if (left & 1) act(left++, f, length);
            if (right & 1) act(--right, f, length);
            left >>= 1; right >>= 1; length *= 2;
        }
        for (std::size_t level = 1; level <= height_; ++level) {
            const auto span = std::size_t{1} << level;
            if (begin & (span - 1)) pull(begin >> level);
            if (end & (span - 1)) pull((end - 1) >> level);
        }
    }
    S prod(int l, int r) {
        check_range(l, r);
        if (l == r) return value_identity_;
        auto begin = base_ + static_cast<std::size_t>(l);
        auto end = base_ + static_cast<std::size_t>(r);
        push_boundary(begin, end);
        // コピー専用型に、局所値を返す際の暗黙ムーブを要求しない。
        if constexpr (!std::is_move_constructible_v<S>) {
            S left = value_identity_, right = value_identity_;
            for (; begin < end; begin >>= 1, end >>= 1) {
                if (begin & 1) left = std::invoke(op, left, data_[begin++]);
                if (end & 1) right = std::invoke(op, data_[--end], right);
            }
            return std::invoke(op, left, right);
        } else {
            auto step = begin & (~begin + 1);
            if (step > end - begin) {
                S result = data_[aligned_node(end) - 1];
                end -= end & (~end + 1);
                while (begin < end) {
                    result = std::invoke(op, data_[aligned_node(end) - 1], result);
                    end -= end & (~end + 1);
                }
                return result;
            }
            S left = data_[aligned_node(begin)];
            begin += step;
            while (begin < end && (step = begin & (~begin + 1)) <= end - begin) {
                left = std::invoke(op, left, data_[aligned_node(begin)]);
                begin += step;
            }
            if (begin == end) return left;
            S right = data_[aligned_node(end) - 1];
            end -= end & (~end + 1);
            while (begin < end) {
                right = std::invoke(op, data_[aligned_node(end) - 1], right);
                end -= end & (~end + 1);
            }
            return std::invoke(op, left, right);
        }
    }

    S all_prod() { return n_ ? data_[1] : value_identity_; }
    template<bool (*pred)(S)> int max_right(int l) {
        return max_right(l, pred);
    }
    template<class Pred> int max_right(int l, Pred pred) {
        check_range(l, l);
        if (static_cast<std::size_t>(l) == n_) return size();
        auto node = base_ + l;
        push_path(node);
        std::size_t length = 1;
        S accumulated = value_identity_;
        while (true) {
            while (!(node & 1)) { node >>= 1; length *= 2; }
            S candidate = std::invoke(op, accumulated, data_[node]);
            if (!pred(candidate)) {
                while (node < base_) {
                    push(node, length); node *= 2; length /= 2;
                    S next = std::invoke(op, accumulated, data_[node]);
                    if (pred(next)) { accumulated = std::move(next); ++node; }
                }
                return static_cast<int>(std::min(n_, node - base_));
            }
            accumulated = std::move(candidate);
            ++node;
            if ((node & (node - 1)) == 0) return size();
        }
    }
    template<bool (*pred)(S)> int min_left(int r) {
        return min_left(r, pred);
    }
    template<class Pred> int min_left(int r, Pred pred) {
        check_range(r, r);
        if (r == 0) return 0;
        auto node = base_ + r;
        push_path(node - 1);
        std::size_t length = 1;
        S accumulated = value_identity_;
        while (true) {
            --node;
            while (node > 1 && (node & 1)) { node >>= 1; length *= 2; }
            S candidate = std::invoke(op, data_[node], accumulated);
            if (!pred(candidate)) {
                while (node < base_) {
                    push(node, length); node = 2 * node + 1; length /= 2;
                    S next = std::invoke(op, data_[node], accumulated);
                    if (pred(next)) { accumulated = std::move(next); --node; }
                }
                return static_cast<int>(node + 1 - base_);
            }
            accumulated = std::move(candidate);
            if ((node & (node - 1)) == 0) return 0;
        }
    }
    std::vector<S> values() {
        if (!n_) return {};
        for (std::size_t begin = 1, length = base_; begin < base_; begin *= 2, length /= 2)
            for (auto p = begin; p < 2 * begin; ++p) push(p, length);
        return std::vector<S>(data_.begin() + base_, data_.begin() + base_ + n_);
    }
    void assign(const std::vector<S>& values) {
        lazy_segtree replacement(values); swap(replacement);
    }
    void reset(int n) {
        lazy_segtree replacement(n); swap(replacement);
    }
    void reset() {
        std::fill(data_.begin(), data_.end(), value_identity_);
        std::fill(lazy_.begin(), lazy_.end(), update_identity_);
        std::fill(pending_.begin(), pending_.end(), 0);
    }
    void clear() noexcept {
        data_.clear(); lazy_.clear(); pending_.clear(); n_ = 0; base_ = 1; height_ = 0;
    }
    void swap(lazy_segtree& other) noexcept(std::is_nothrow_swappable_v<S> && std::is_nothrow_swappable_v<F>) {
        using std::swap;
        swap(value_identity_, other.value_identity_); swap(update_identity_, other.update_identity_);
        swap(n_, other.n_); swap(base_, other.base_); swap(height_, other.height_);
        data_.swap(other.data_); lazy_.swap(other.lazy_); pending_.swap(other.pending_);
    }
};
#endif

#ifndef SIRKIT_BINARY_RANGE_STATISTICS_DEFINED
#define SIRKIT_BINARY_RANGE_STATISTICS_DEFINED

struct BinaryRunStatistics
{
    int length = 0;
    int ones = 0;
    std::array<int, 2> prefix{};
    std::array<int, 2> suffix{};
    std::array<int, 2> longest{};
};

namespace detail
{
constexpr BinaryRunStatistics binary_run_identity()
{
    return {};
}

constexpr BinaryRunStatistics binary_run_uniform(int length, bool bit)
{
    BinaryRunStatistics result;
    result.length = length;
    result.ones = bit ? length : 0;
    result.prefix[bit] = result.suffix[bit] = result.longest[bit] = length;
    return result;
}

constexpr BinaryRunStatistics binary_run_merge(const BinaryRunStatistics &a, const BinaryRunStatistics &b)
{
    BinaryRunStatistics result;
    result.length = a.length + b.length;
    result.ones = a.ones + b.ones;
    for (int bit = 0; bit < 2; ++bit)
    {
        result.prefix[bit] = a.prefix[bit] + (a.prefix[bit] == a.length ? b.prefix[bit] : 0);
        result.suffix[bit] = b.suffix[bit] + (b.suffix[bit] == b.length ? a.suffix[bit] : 0);
        result.longest[bit] = std::max({a.longest[bit], b.longest[bit], a.suffix[bit] + b.prefix[bit]});
    }
    return result;
}

// fのbit0/bit1は入力が0/1のときに返す値。
inline constexpr std::uint8_t kBinaryRunSetZero = 0;
inline constexpr std::uint8_t kBinaryRunFlip = 1;
inline constexpr std::uint8_t kBinaryRunIdentity = 2;
inline constexpr std::uint8_t kBinaryRunSetOne = 3;

constexpr std::uint8_t binary_run_no_change()
{
    return kBinaryRunIdentity;
}

constexpr std::uint8_t binary_run_compose(std::uint8_t newer, std::uint8_t older)
{
    const unsigned zero = (newer >> (older & 1)) & 1;
    const unsigned one = (newer >> (older >> 1)) & 1;
    return static_cast<std::uint8_t>(zero | (one << 1));
}

constexpr BinaryRunStatistics binary_run_map(std::uint8_t action, BinaryRunStatistics value)
{
    if (action == kBinaryRunIdentity)
    {
        return value;
    }
    if (action == kBinaryRunFlip)
    {
        value.ones = value.length - value.ones;
        std::swap(value.prefix[0], value.prefix[1]);
        std::swap(value.suffix[0], value.suffix[1]);
        std::swap(value.longest[0], value.longest[1]);
        return value;
    }
    return binary_run_uniform(value.length, action == kBinaryRunSetOne);
}
}

class BinaryRangeStatistics
{
    using Tree = lazy_segtree<BinaryRunStatistics, detail::binary_run_merge, detail::binary_run_identity,
                              std::uint8_t, detail::binary_run_map, detail::binary_run_compose, detail::binary_run_no_change>;

public:
    explicit BinaryRangeStatistics(std::string_view bits = {}) : tree_(MakeLeaves(bits))
    {
    }

    explicit BinaryRangeStatistics(int n) : tree_(ZeroLeaves(n))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return tree_.size();
    }

    void flip(int left, int right)
    {
        tree_.apply(left, right, detail::kBinaryRunFlip);
    }

    void assign(int left, int right, bool bit)
    {
        tree_.apply(left, right, bit ? detail::kBinaryRunSetOne : detail::kBinaryRunSetZero);
    }

    [[nodiscard]] BinaryRunStatistics stats(int left, int right)
    {
        return tree_.prod(left, right);
    }

    [[nodiscard]] BinaryRunStatistics all_stats()
    {
        return tree_.all_prod();
    }

private:
    Tree tree_;

    static std::vector<BinaryRunStatistics> MakeLeaves(std::string_view bits)
    {
        detail::CheckedSegmentArrayCount(bits.size(), "BinaryRangeStatistics: size");
        std::vector<BinaryRunStatistics> leaves;
        leaves.reserve(bits.size());
        for (const char bit : bits)
        {
            if (bit != '0' && bit != '1')
            {
                throw std::invalid_argument("BinaryRangeStatistics: invalid bit");
            }
            leaves.push_back(detail::binary_run_uniform(1, bit == '1'));
        }
        return leaves;
    }

    static std::vector<BinaryRunStatistics> ZeroLeaves(int size)
    {
        if (size < 0)
        {
            throw std::length_error("BinaryRangeStatistics: negative size");
        }
        return std::vector<BinaryRunStatistics>(static_cast<std::size_t>(size), detail::binary_run_uniform(1, false));
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
