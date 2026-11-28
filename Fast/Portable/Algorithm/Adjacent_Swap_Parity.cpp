// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Adjacent_Swap_Parity.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/adjacent_swap_parity.md
// 参考（同値の要素の対応と交換回数の偶奇）: https://atcoder.jp/contests/arc214/editorial/15190
//
// API一覧
// adjacent_swap_distances_by_parity(from,to,compare={}): 隣接交換で一致させる最小回数を偶数・奇数別に返す。
// AdjacentSwapParityDistances{even,odd}: それぞれoptional<uint64_t>。その偶奇で到達できないときnullopt。
// 同値な2要素の交換も1回と数える。比較・列の要件はadjacent_swap_distanceと同じ、長さn<=INT_MAX。
// 計算量はO((n+1)log(n+2))時間・O(n+1)領域。
//
// [同梱の隣接交換距離]
// adjacent_swap_permutation(from,to,compare={}): from[i]に対応するtoの位置を添字iの順に返す。
// 同じ値の出現は先頭から順に対応させる。添字は0始まり。
// adjacent_swap_distance(from,to,compare={}): 隣接する2要素の交換でfromをtoへ変える最小回数。
// 変換できない場合はいずれもnullopt。compareで互いに小さくない値を同値と扱う。
// std::size・添字アクセスに対応する列で使用可。長さnはUINT32_MAX以下。O((n+1)log(n+2))時間・O(n+1)領域。
//
// [同梱の転倒数]
// inversion_count(values): i<jかつvalues[j]<values[i]の組数。O(n log(n+1))時間・O(n)領域。
// range_inversion_counts(values,ranges): 各半開区間[l,r)の転倒数を入力順で返す。
// ブロック幅をbとし、O((n log(n+1)+q log(q+1))+(n²/b+qb)log(n+1))時間・O(n+q)領域。
// n≤UINT32_MAX。比較は狭義弱順序。空区間・重複値を扱う。
//
// [同梱のFenwick木]
// [fenwick_tree<T>]
// fenwick_tree()/fenwick_tree(int n)/fenwick_tree(vector<T>): 空列/n要素の0/列から構築。O(n+1)時間・領域。
// add(int p,T x): 点加算。sum(int l,int r): 半開区間[l,r)の和をT値で返す。O(log(n+1))、空区間はT{}。
// 点添字は0<=p<size()、区間は0<=l<=r<=size()。
// ACL形式の公開APIをsirkit内に定義する。bool以外の整数は内部を符号なしにして型幅のmod 2^bitで計算する。
// その他のTはT{}が単位元の可換加法群とし、加減算・コピーをO(1)とする。要素数はintの範囲内。
// size()/empty(): 要素数/空か、O(1)。get(int p)/set(int p,T value): 点取得/代入、O(log(n+1))。
// lower_bound(target)/upper_bound(target): sum(0,i+1)>=target/>targetとなる最小int添字、なければsize()。O(log(n+1))。
// 境界探索は全要素が非負で、途中の和がTに収まり、加算と順序が整合すること。全要素の検査はしない。
// values(): 列を復元して返す。O(n+1)時間・追加O(n)。reset(int n)/reset(): n要素/現在の長さを0にする。O(旧n+n+1)。
// clear()/swap(other): 空にして容量保持/交換。O(旧n+1)/O(1)。コピーは独立、移動元は空として使える。
// 添字・区間は既定で検査しSKIP_RUNTIME_CHECKSで除く。容量・添字のオーバーフロー対策は残す。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
#include <functional>
#include <iterator>
#include <numeric>
#include <optional>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_INVERSIONS_DEFINED
#define SIRKIT_RANGE_INVERSIONS_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void check_range_inversion_size(std::uint64_t size)
{
    if (size > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::length_error("range_inversion_counts: too many elements");
    }
}
} // namespace detail

template <class T>
std::vector<std::uint64_t> range_inversion_counts(
    const std::vector<T> &values, std::span<const std::pair<std::size_t, std::size_t>> ranges)
{
    const auto n = values.size();
    detail::check_range_inversion_size(n);
    std::vector<std::uint64_t> result(ranges.size());
    if (ranges.empty())
    {
        return result;
    }

    struct Query
    {
        std::size_t left, right, index, block;
    };
    const auto block_size = std::max<std::size_t>(1, static_cast<std::size_t>(n / std::sqrt(static_cast<long double>(ranges.size()))));
    std::vector<Query> queries;
    queries.reserve(ranges.size());
    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
        const auto [left, right] = ranges[i];
        if (left != right)
        {
            queries.push_back({left, right, i, left / block_size});
        }
    }
    if (queries.empty())
    {
        return result;
    }
    std::ranges::sort(queries, [](const Query &a, const Query &b)
                      {
                          if (a.block != b.block)
                          {
                              return a.block < b.block;
                          }
                          return a.block % 2 == 0 ? a.right < b.right : b.right < a.right;
                      });

    auto sorted = values;
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end(), [](const T &a, const T &b)
                             {
                                 return (a < b) == (b < a);
                             }),
                 sorted.end());
    std::vector<std::uint32_t> rank;
    rank.reserve(n);
    for (const auto &value : values)
    {
        rank.push_back(static_cast<std::uint32_t>(std::lower_bound(sorted.begin(), sorted.end(), value) - sorted.begin()));
    }

    std::vector<std::uint32_t> tree(sorted.size());
    const auto add = [&](std::size_t index, std::uint32_t delta)
    {
        for (; index < tree.size(); index |= index + 1)
        {
            tree[index] += delta;
        }
    };
    const auto count = [&](std::size_t end)
    {
        std::uint32_t total = 0;
        for (; end != 0; end &= end - 1)
        {
            total += tree[end - 1];
        }
        return total;
    };
    std::uint64_t inversions = 0;
    std::size_t left = 0, right = 0;
    for (const auto &[wanted_left, wanted_right, index, block] : queries)
    {
        while (left > wanted_left)
        {
            const auto r = rank[--left];
            inversions += count(r);
            add(r, 1);
        }
        while (right < wanted_right)
        {
            const auto r = rank[right];
            inversions += right - left - count(static_cast<std::size_t>(r) + 1);
            add(r, 1);
            ++right;
        }
        while (left < wanted_left)
        {
            const auto r = rank[left++];
            inversions -= count(r);
            add(r, std::numeric_limits<std::uint32_t>::max());
        }
        while (right > wanted_right)
        {
            const auto r = rank[--right];
            inversions -= right - left + 1 - count(static_cast<std::size_t>(r) + 1);
            add(r, std::numeric_limits<std::uint32_t>::max());
        }
        result[index] = inversions;
    }
    return result;
}

template <class T>
std::uint64_t inversion_count(const std::vector<T> &values)
{
    const std::pair<std::size_t, std::size_t> range{0, values.size()};
    return range_inversion_counts(values, {&range, 1}).front();
}

#endif

#ifndef SIRKIT_ADJACENT_SWAP_DISTANCE_DEFINED
#define SIRKIT_ADJACENT_SWAP_DISTANCE_DEFINED

template<class From, class To, class Compare = std::less<>>
std::optional<std::vector<std::size_t>> adjacent_swap_permutation(const From &from, const To &to, Compare compare = {})
{
    const auto n = static_cast<std::size_t>(std::size(from));
    if (n != static_cast<std::size_t>(std::size(to)))
    {
        return std::nullopt;
    }
    detail::check_range_inversion_size(n);
    const auto sorted_indices = [&](const auto &values) {
        std::vector<std::size_t> order(n);
        std::iota(order.begin(), order.end(), std::size_t{0});
        std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
            if (std::invoke(compare, values[a], values[b]))
            {
                return true;
            }
            if (std::invoke(compare, values[b], values[a]))
            {
                return false;
            }
            return a < b;
        });
        return order;
    };
    const auto sources = sorted_indices(from);
    const auto targets = sorted_indices(to);
    std::vector<std::size_t> permutation(n);
    for (std::size_t rank = 0; rank < n; ++rank)
    {
        const auto first = sources[rank];
        const auto second = targets[rank];
        if (std::invoke(compare, from[first], to[second]) || std::invoke(compare, to[second], from[first]))
        {
            return std::nullopt;
        }
        permutation[first] = second;
    }
    return permutation;
}

template<class From, class To, class Compare = std::less<>>
std::optional<std::uint64_t> adjacent_swap_distance(const From &from, const To &to, Compare compare = {})
{
    const auto permutation = adjacent_swap_permutation(from, to, std::move(compare));
    if (!permutation)
    {
        return std::nullopt;
    }
    // 同値な要素どうしの順序を保った対応なら、余分な交差がない。
    return inversion_count(*permutation);
}
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <algorithm>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <bit>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <type_traits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_HLD_FENWICK_ACCESS_FORWARD_DEFINED
#define SIRKIT_HLD_FENWICK_ACCESS_FORWARD_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ { template<class Range> struct HldFenwickAccess; }
#endif

#ifndef SIRKIT_FENWICK_STORAGE_TYPE_DEFINED
#define SIRKIT_FENWICK_STORAGE_TYPE_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class T, bool = std::is_integral_v<T> && !std::is_same_v<T, bool>>
struct FenwickStorage { using type = T; };
template<class T>
struct FenwickStorage<T, true> { using type = std::make_unsigned_t<T>; };
#if defined(__SIZEOF_INT128__)
template<> struct FenwickStorage<__int128> { using type = unsigned __int128; };
template<> struct FenwickStorage<unsigned __int128> { using type = unsigned __int128; };
#endif
}
#endif

#ifndef SIRKIT_FENWICK_TREE_DEFINED
#define SIRKIT_FENWICK_TREE_DEFINED
namespace detail {
inline std::size_t CheckedFenwickStorageSize(std::size_t n, std::size_t limit) {
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()) || n >= limit)
        throw std::length_error("fenwick_tree: size");
    return n == 0 ? 0 : n + 1;
}
}

template<class T>
class fenwick_tree {
    // HLD所有時の読取り専用接続。通常のFenwick操作には使わない。
    using HldFenwickTag = void;
    template<class> friend struct /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldFenwickAccess;
    using Storage = typename /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FenwickStorage<T>::type;
    std::vector<Storage> tree_;
    static decltype(auto) to_storage(const T& value) {
        if constexpr (std::is_same_v<T, Storage>) return (value);
        else return static_cast<Storage>(value);
    }
    static decltype(auto) from_storage(Storage& value) {
        if constexpr (std::is_same_v<T, Storage>) return (value);
        else return static_cast<T>(value);
    }
    static std::size_t checked_size(int n) {
        return static_cast<std::size_t>(n);
    }
    static std::size_t storage_size(std::size_t n) {
        return detail::CheckedFenwickStorageSize(n, std::vector<Storage>{}.max_size());
    }
    static std::size_t lowbit(std::size_t x) noexcept { return x & (~x + 1); }
    void check_index(int i) const {
        (void)i;
    }
    void check_range(int l, int r) const {
        (void)l; (void)r;
    }
    Storage prefix_unchecked(std::size_t r) const {
        Storage result{};
        for (; r; r -= lowbit(r)) result += tree_[r];
        return result;
    }
    Storage get_unchecked(std::size_t i) const {
        auto node = i + 1;
        Storage result = tree_[node];
        const auto begin = node - lowbit(node);
        for (--node; node > begin; node -= lowbit(node)) result -= tree_[node];
        return result;
    }
    void add_unchecked(std::size_t i, const Storage& delta) {
        const auto n = static_cast<std::size_t>(size());
        for (++i;;) {
            tree_[i] += delta;
            const auto step = lowbit(i);
            if (step > n - i) break;
            i += step;
        }
    }
public:
    fenwick_tree() = default;
    explicit fenwick_tree(int n) : tree_(storage_size(checked_size(n)), Storage{}) {}
    template<class Values> requires std::is_same_v<Values, std::vector<T>>
    explicit fenwick_tree(const Values& values) : tree_(storage_size(values.size()), Storage{}) {
        const auto n = values.size();
        for (std::size_t i = 1; i <= n; ++i) {
            tree_[i] += to_storage(values[i - 1]);
            const auto step = lowbit(i);
            if (step <= n - i) tree_[i + step] += tree_[i];
        }
    }
    fenwick_tree(const fenwick_tree&) = default;
    fenwick_tree& operator=(const fenwick_tree&) = default;
    fenwick_tree(fenwick_tree&& other) noexcept : tree_(std::move(other.tree_)) { other.tree_.clear(); }
    fenwick_tree& operator=(fenwick_tree&& other) noexcept {
        if (this != &other) { tree_ = std::move(other.tree_); other.tree_.clear(); }
        return *this;
    }
    int size() const noexcept { return tree_.empty() ? 0 : static_cast<int>(tree_.size() - 1); }
    bool empty() const noexcept { return size() == 0; }
    void add(int i, T delta) {
        check_index(i);
        add_unchecked(static_cast<std::size_t>(i), to_storage(delta));
    }
    T get(int i) const { check_index(i); return static_cast<T>(get_unchecked(static_cast<std::size_t>(i))); }
    void set(int i, T value) {
        check_index(i);
        const Storage delta = to_storage(value) - get_unchecked(static_cast<std::size_t>(i));
        add_unchecked(static_cast<std::size_t>(i), delta);
    }
    T sum(int l, int r) {
        check_range(l, r);
        return static_cast<T>(prefix_unchecked(static_cast<std::size_t>(r)) - prefix_unchecked(static_cast<std::size_t>(l)));
    }
    int lower_bound(const T& target) const {
        if (!(T{} < target)) return 0;
        const auto n = static_cast<std::size_t>(size());
        std::size_t index = 0;
        Storage current{};
        for (auto jump = std::bit_floor(n); jump; jump >>= 1) {
            if (jump <= n - index) {
                const auto next = index + jump;
                Storage candidate = current + tree_[next];
                if (from_storage(candidate) < target) { current = std::move(candidate); index = next; }
            }
        }
        return static_cast<int>(index);
    }
    int upper_bound(const T& target) const {
        if (target < T{}) return 0;
        const auto n = static_cast<std::size_t>(size());
        std::size_t index = 0;
        Storage current{};
        for (auto jump = std::bit_floor(n); jump; jump >>= 1) {
            if (jump <= n - index) {
                const auto next = index + jump;
                Storage candidate = current + tree_[next];
                if (!(target < from_storage(candidate))) { current = std::move(candidate); index = next; }
            }
        }
        return static_cast<int>(index);
    }
    std::vector<T> values() const {
        if (tree_.empty()) return {};
        std::vector<T> result(tree_.begin() + 1, tree_.end());
        const auto n = static_cast<std::size_t>(size());
        for (auto i = n; i; --i) {
            const auto step = lowbit(i);
            if (step <= n - i) {
                if constexpr (std::is_same_v<T, Storage>) result[i + step - 1] -= result[i - 1];
                else {
                    const Storage difference = static_cast<Storage>(result[i + step - 1]) - static_cast<Storage>(result[i - 1]);
                    result[i + step - 1] = static_cast<T>(difference);
                }
            }
        }
        return result;
    }
    void reset(int n) {
        std::vector<Storage> replacement(storage_size(checked_size(n)), Storage{});
        tree_.swap(replacement);
    }
    void reset() { std::fill(tree_.begin(), tree_.end(), Storage{}); }
    void clear() noexcept { tree_.clear(); }
    void swap(fenwick_tree& other) noexcept { tree_.swap(other.tree_); }
};
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <algorithm>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <functional>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <iterator>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <numeric>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <optional>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_ADJACENT_SWAP_PARITY_DEFINED
#define SIRKIT_ADJACENT_SWAP_PARITY_DEFINED

struct AdjacentSwapParityDistances
{
    std::optional<std::uint64_t> even;
    std::optional<std::uint64_t> odd;
};

template <class From, class To, class Compare = std::less<>>
[[nodiscard]] AdjacentSwapParityDistances adjacent_swap_distances_by_parity(const From &from, const To &to, Compare compare = {})
{
    const auto n = static_cast<std::size_t>(std::size(from));
    if (n != static_cast<std::size_t>(std::size(to)))
    {
        return {};
    }
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("adjacent_swap_distances_by_parity: input too long");
    }
    const auto matching = adjacent_swap_permutation(from, to, std::ref(compare));
    if (!matching)
    {
        return {};
    }
    const auto &permutation = *matching;
    std::vector<std::size_t> sorted(n);
    std::iota(sorted.begin(), sorted.end(), std::size_t{0});
    std::ranges::sort(sorted, [&](std::size_t a, std::size_t b)
                      {
                          if (std::invoke(compare, from[a], from[b]))
                          {
                              return true;
                          }
                          if (std::invoke(compare, from[b], from[a]))
                          {
                              return false;
                          }
                          return a < b;
                      });
    std::vector<std::size_t> before(n, n), after(n, n);
    for (std::size_t i = 1; i < n; ++i)
    {
        const auto l = sorted[i - 1], r = sorted[i];
        if (!std::invoke(compare, from[l], from[r]))
        {
            before[r] = l;
            after[l] = r;
        }
    }

    fenwick_tree<std::uint32_t> seen(static_cast<int>(n));
    const auto between_values = [&](std::size_t l, std::size_t r)
    {
        return seen.sum(static_cast<int>(permutation[l] + 1), static_cast<int>(permutation[r]));
    };
    std::vector<std::uint32_t> before_left(n);
    std::uint64_t minimum = 0;
    auto additional = std::numeric_limits<std::uint64_t>::max();
    for (std::size_t i = 0; i < n; ++i)
    {
        const auto value = static_cast<int>(permutation[i]);
        minimum += i - seen.sum(0, value);
        if (before[i] != n)
        {
            const auto inside = between_values(before[i], i) - before_left[i]; // 端点を交差させると、内部の対応点ごとに2転倒増える。
            additional = std::min(additional, 2 * std::uint64_t{inside} + 1);
        }
        if (after[i] != n)
        {
            before_left[after[i]] = between_values(i, after[i]);
        }
        seen.add(value, 1);
    }

    std::optional<std::uint64_t> opposite;
    if (additional != std::numeric_limits<std::uint64_t>::max())
    {
        opposite = minimum + additional;
    }
    if ((minimum & 1u) == 0)
    {
        return {minimum, opposite};
    }
    return {opposite, minimum};
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
