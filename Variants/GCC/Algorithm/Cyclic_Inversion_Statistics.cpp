// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Algorithm/Cyclic_Inversion_Statistics.cpp
// 参考（巡回シフトの転倒数の差分）: https://atcoder.jp/contests/arc218/editorial/19876
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/cyclic_inversion_statistics.md
//
// API一覧
// CyclicInversionStatistics(values,less={}): 列の比較順位から構築。O(n*log(n+1)+1)時間・O(n+1)領域。
// valuesは長さが分かるランダムアクセス範囲。lessは既定でstd::less<>。同値の組は転倒に含まない。
// inversion_count(): 現在の転倒数。rotation_inversion_sum(): 全n個の左巡回シフトの転倒数の和。各O(1)。
// left_rotation_counts(): 0回からn-1回の左巡回シフトの転倒数を順に返す。O(n+1)時間・領域。
// swap_adjacent(i): 0始まりの位置iとi+1を交換。O(1)。末尾と先頭の交換は扱わない。
// size()/empty(): 長さ/空か。O(1)。集計値はint64_t。n<=2642246、空構築可、ムーブ元は空。
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
#include <bit>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include <cstdint>
#include <functional>
#include <iterator>
#include <numeric>
#include <ranges>


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
#ifndef SKIP_RUNTIME_CHECKS
        if (n < 0) throw std::invalid_argument("fenwick_tree: negative size");
#endif
        return static_cast<std::size_t>(n);
    }
    static std::size_t storage_size(std::size_t n) {
        return detail::CheckedFenwickStorageSize(n, std::vector<Storage>{}.max_size());
    }
    static std::size_t lowbit(std::size_t x) noexcept { return x & (~x + 1); }
    void check_index(int i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (i < 0 || i >= size()) throw std::out_of_range("fenwick_tree: index");
#else
        (void)i;
#endif
    }
    void check_range(int l, int r) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (l < 0 || l > r || r > size()) throw std::out_of_range("fenwick_tree: range");
#else
        (void)l; (void)r;
#endif
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

#ifndef SIRKIT_CYCLIC_INVERSION_STATISTICS_DEFINED
#define SIRKIT_CYCLIC_INVERSION_STATISTICS_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline constexpr std::size_t kMaxCyclicInversionLength = 2'642'246;

inline void check_cyclic_inversion_length(std::size_t n)
{
    // n個の転倒数をすべて足した上限 n*n*(n-1)/2 が int64_t に収まる。
    if (n > kMaxCyclicInversionLength)
    {
        throw std::length_error("CyclicInversionStatistics: input too long");
    }
}
} // namespace reinsirk::detail

class CyclicInversionStatistics
{
    std::vector<int> ranks_;
    std::vector<int> rotation_changes_;
    std::int64_t inversions_ = 0;
    std::int64_t rotation_sum_ = 0;

    template <class Visitor>
    void visit_left_rotations(Visitor visit) const
    {
        auto current = inversions_;
        for (const auto rank : ranks_)
        {
            visit(current);
            current += rotation_changes_[static_cast<std::size_t>(rank)];
        }
    }

public:
    CyclicInversionStatistics() = default;

    template <class Range, class Compare = std::less<>>
        requires std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range>
            && std::indirect_strict_weak_order<Compare, std::ranges::iterator_t<const Range>>
    explicit CyclicInversionStatistics(const Range &values, Compare less = {})
    {
        const auto count = static_cast<std::size_t>(std::ranges::size(values));
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::check_cyclic_inversion_length(count);
        const int n = static_cast<int>(count);
        std::vector<int> positions(count);
        std::iota(positions.begin(), positions.end(), 0);
        ranks_.resize(count);
        rotation_changes_.reserve(count);
        const auto first = std::ranges::begin(values);
        const auto position_less = [&](int a, int b)
        {
            return std::invoke(less, first[a], first[b]);
        };
        std::sort(positions.begin(), positions.end(), position_less);
        for (std::size_t begin = 0; begin < count;)
        {
            std::size_t end = begin + 1;
            while (end < count && !position_less(positions[begin], positions[end]))
            {
                ++end;
            }
            const int rank = static_cast<int>(rotation_changes_.size());
            // この値を先頭から末尾へ移すと、前後が逆になるのは大小が異なる組だけ。
            rotation_changes_.push_back(n - static_cast<int>(end) - static_cast<int>(begin));
            for (std::size_t i = begin; i < end; ++i)
            {
                ranks_[static_cast<std::size_t>(positions[i])] = rank;
            }
            begin = end;
        }

        fenwick_tree<int> seen(static_cast<int>(rotation_changes_.size()));
        for (int i = 0; i < n; ++i)
        {
            const int rank = ranks_[static_cast<std::size_t>(i)];
            inversions_ += i - seen.sum(0, rank + 1);
            seen.add(rank, 1);
        }
        visit_left_rotations([&](std::int64_t value)
        {
            rotation_sum_ += value;
        });
    }

    CyclicInversionStatistics(const CyclicInversionStatistics &) = default;

    CyclicInversionStatistics(CyclicInversionStatistics &&other) noexcept
        : ranks_(std::move(other.ranks_)), rotation_changes_(std::move(other.rotation_changes_)),
          inversions_(std::exchange(other.inversions_, 0)), rotation_sum_(std::exchange(other.rotation_sum_, 0))
    {
        other.ranks_.clear();
        other.rotation_changes_.clear();
    }

    CyclicInversionStatistics &operator=(CyclicInversionStatistics other) noexcept
    {
        ranks_.swap(other.ranks_);
        rotation_changes_.swap(other.rotation_changes_);
        std::swap(inversions_, other.inversions_);
        std::swap(rotation_sum_, other.rotation_sum_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return ranks_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return ranks_.empty();
    }

    [[nodiscard]] std::int64_t inversion_count() const noexcept
    {
        return inversions_;
    }

    [[nodiscard]] std::int64_t rotation_inversion_sum() const noexcept
    {
        return rotation_sum_;
    }

    [[nodiscard]] std::vector<std::int64_t> left_rotation_counts() const
    {
        std::vector<std::int64_t> result;
        result.reserve(size());
        visit_left_rotations([&](std::int64_t value)
        {
            result.push_back(value);
        });
        return result;
    }

    // 隣り合う二つの回転差分の重みは1だけ異なる。
    void swap_adjacent(std::size_t position)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (position >= size() || position + 1 >= size())
        {
            throw std::out_of_range("CyclicInversionStatistics: adjacent positions");
        }
#endif
        const auto first = ranks_[position];
        const auto second = ranks_[position + 1];
        const int difference = (first < second) - (first > second);
        inversions_ += difference;
        rotation_sum_ += static_cast<std::int64_t>(size()) * difference
            + rotation_changes_[static_cast<std::size_t>(second)] - rotation_changes_[static_cast<std::size_t>(first)];
        std::swap(ranks_[position], ranks_[position + 1]);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
