// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Union_Find_Order_Statistics.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/union_find_order_statistics.md
// 同梱UnionFindの出典: https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/union-find
//
// API一覧
// UnionFindOrderStatistics<T>(labels): 初期ラベルを固定し、全頂点を独立成分にする。頂点は0始まり。
// Tはコピー可能な全順序型。異なる値は座標圧縮し、同じ値は多重度に数える。
// merge(a,b): 新規結合ならtrue。全結合での木の併合時間はO(n log(m+1))。成分探索は償却O(α(n))。
// root(v)/issame(a,b)/size(v): 代表/同一成分/成分サイズ。償却O(α(n))。
// size()/component_count(): 全頂点数/成分数。O(1)。
// kth(v,k): 成分内のラベルの昇順で0始まりk番目。範囲外はnullopt。
// count(v,label)/count_less(v,label): 成分内で同値/未満のラベル数。
// 照会はTの比較とコピーをO(1)としてO(log(m+1)+α(n))償却。
// nは頂点数でINT_MAX以下、mは異なるラベルの個数。構築O(n log(n+1))時間・O(n log(m+1))領域。
//
// [同梱のUnionFind]
// [UnionFind]
// UnionFind(n=0): n要素を独立成分として構築。O(n+1)時間・領域、n<=INT_MAX。
// root(i)/leader(i): 代表、unite(a,b): 結合（戻り値なし）、merge(a,b): 新規結合ならtrue。償却O(α(n+1))。
// issame(a,b)/same(a,b): 同一成分か、size(i): 成分サイズ。償却O(α(n+1))。探索時に経路を圧縮する。
// size()/empty()/component_count(): 全要素数/空か/成分数、O(1)。groups(): 頂点昇順の成分一覧、O(1+n α(n+1))償却。
// reset(n)/reset(): n要素/現在の要素数へ初期化。容量を再利用、O(旧n+n+1)上界。確保失敗時は元を保つ。
// clear(): 空へ戻して容量保持、O(旧n+1)上界、noexcept。swap(other): 内容交換、O(1)、noexcept。
// コピーはO(n+1)で独立、移動元は空。自己代入・自己ムーブは内容を保つ。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNION_FIND_DEFINED
#define SIRKIT_UNION_FIND_DEFINED
class UnionFind {
    std::vector<int> param;
    int components_ = 0;
    static std::size_t checked_size(std::size_t n) {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("UnionFind: too many vertices");
        return n;
    }
    void check(int i) const {
        (void)i;
    }
    int leader_unchecked(int i) {
        while (param[i] >= 0) {
            const int p = param[i];
            if (param[p] < 0) return p;
            i = param[i] = param[p];
        }
        return i;
    }
public:
    explicit UnionFind(std::size_t n = 0)
        : param(checked_size(n), -1), components_(static_cast<int>(n)) {}
    UnionFind(const UnionFind&) = default;
    UnionFind& operator=(const UnionFind&) = default;
    UnionFind(UnionFind&& other) noexcept
        : param(std::move(other.param)), components_(std::exchange(other.components_, 0)) { other.param.clear(); }
    UnionFind& operator=(UnionFind&& other) noexcept {
        if (this != &other) {
            param = std::move(other.param);
            components_ = std::exchange(other.components_, 0); other.param.clear();
        }
        return *this;
    }
    void reset(std::size_t n) {
        const auto count = checked_size(n);
        param.reserve(count);
        param.clear(); param.resize(count, -1);
        components_ = static_cast<int>(count);
    }
    void reset() { reset(param.size()); }
    void clear() noexcept { param.clear(); components_ = 0; }
    bool empty() const noexcept { return param.empty(); }
    void swap(UnionFind& other) noexcept {
        param.swap(other.param); std::swap(components_, other.components_);
    }
    friend void swap(UnionFind& left, UnionFind& right) noexcept { left.swap(right); }
    int root(int i) { check(i); return leader_unchecked(i); }
    int leader(int i) { return root(i); }
    bool merge(int a, int b) {
        check(a); check(b);
        // 経路圧縮は根の負サイズを変えないので、第二探索後も再利用できる。
        int sa = param[a];
        while (sa >= 0) {
            const int next = param[sa];
            if (next < 0) { a = sa; sa = next; break; }
            a = param[a] = next; sa = param[a];
        }
        // 根自身・根直下なら第二探索の書戻しは発生しない。
        if (a == b) return false;
        int sb = param[b];
        while (sb >= 0) {
            if (sb == a) return false;
            const int next = param[sb];
            if (next < 0) { b = sb; sb = next; break; }
            b = param[b] = next; sb = param[b];
        }
        if (a == b) return false;
        if (sa > sb) std::swap(a, b);
        param[a] = sa + sb; param[b] = a; --components_;
        return true;
    }
    void unite(int a, int b) { (void)merge(a, b); }
    bool same(int a, int b) { check(a); check(b); return leader_unchecked(a) == leader_unchecked(b); }
    bool issame(int a, int b) { return same(a, b); }
    int size(int i) { return -param[root(i)]; }
    int size() const noexcept { return static_cast<int>(param.size()); }
    int component_count() const noexcept { return components_; }
    std::vector<std::vector<int>> groups() {
        std::vector<std::vector<int>> result;
        result.reserve(static_cast<std::size_t>(components_));
        if (components_ == size()) {
            for (int v = 0; v < size(); ++v) result.emplace_back(std::size_t{1}, v);
            return result;
        }
        if (components_ == 1) {
            const int root = leader_unchecked(0);
            result.emplace_back(param.size());
            const int negative_size = param[root];
            std::fill(param.begin(), param.end(), root);
            param[root] = negative_size;
            std::iota(result[0].begin(), result[0].end(), 0);
            return result;
        }
        std::vector<int> slot(param.size());
        for (int i = 0; i < size(); ++i) if (param[i] < 0) {
            slot[i] = static_cast<int>(result.size());
            result.emplace_back(); result.back().reserve(static_cast<std::size_t>(-param[i]));
        }
        for (int i = 0; i < size(); ++i) result[slot[leader_unchecked(i)]].push_back(i);
        return result;
    }
};
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
#include <concepts>

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
#include <optional>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_UNION_FIND_ORDER_STATISTICS_DEFINED
#define SIRKIT_UNION_FIND_ORDER_STATISTICS_DEFINED

template <std::totally_ordered T>
    requires std::copyable<T>
class UnionFindOrderStatistics
{
    using Index = std::size_t;

public:
    explicit UnionFindOrderStatistics(const std::vector<T> &labels)
        : groups_(labels.size()), values_(labels), roots_(labels.size())
    {
        std::sort(values_.begin(), values_.end());
        values_.erase(std::unique(values_.begin(), values_.end()), values_.end());
        if (!labels.empty())
        {
            const auto path_nodes = std::bit_width(values_.size() - 1) + 1;
            const auto estimated = static_cast<std::uint64_t>(labels.size()) * path_nodes;
            nodes_.reserve(static_cast<Index>(std::min<std::uint64_t>(estimated, nodes_.max_size())));
        }
        for (Index v = 0; v < labels.size(); ++v)
        {
            roots_[v] = BuildSingleton(0, values_.size(), LowerBound(labels[v]));
        }
    }

    UnionFindOrderStatistics(const UnionFindOrderStatistics &) = default;
    UnionFindOrderStatistics(UnionFindOrderStatistics &&) noexcept = default;

    UnionFindOrderStatistics &operator=(UnionFindOrderStatistics other) noexcept
    {
        groups_.swap(other.groups_);
        values_.swap(other.values_);
        roots_.swap(other.roots_);
        nodes_.swap(other.nodes_);
        return *this;
    }

    bool merge(int a, int b)
    {
        a = groups_.root(a);
        b = groups_.root(b);
        if (!groups_.merge(a, b))
        {
            return false;
        }
        const auto joined = MergeTrees(roots_[a], roots_[b]);
        roots_[a] = joined;
        roots_[b] = joined;
        return true;
    }

    int root(int vertex)
    {
        return groups_.root(vertex);
    }

    bool issame(int a, int b)
    {
        return groups_.issame(a, b);
    }

    int size(int vertex)
    {
        return groups_.size(vertex);
    }

    int size() const noexcept
    {
        return groups_.size();
    }

    int component_count() const noexcept
    {
        return groups_.component_count();
    }

    std::optional<T> kth(int vertex, int k)
    {
        auto id = roots_[groups_.root(vertex)];
        if (k < 0 || k >= SizeOf(id))
        {
            return std::nullopt;
        }
        Index from = 0;
        Index to = values_.size();
        while (to - from > 1)
        {
            const auto &node = nodes_[id - 1];
            const auto mid = from + (to - from) / 2;
            const int left_count = SizeOf(node.left);
            if (k < left_count)
            {
                id = node.left;
                to = mid;
            }
            else
            {
                k -= left_count;
                id = node.right;
                from = mid;
            }
        }
        return values_[from];
    }

    int count(int vertex, const T &label)
    {
        const auto id = roots_[groups_.root(vertex)];
        const auto end = LowerBound(label);
        if (end == values_.size() || values_[end] != label)
        {
            return 0;
        }
        return Prefix(id, 0, values_.size(), end + 1) - Prefix(id, 0, values_.size(), end);
    }

    int count_less(int vertex, const T &label)
    {
        const auto id = roots_[groups_.root(vertex)];
        return Prefix(id, 0, values_.size(), LowerBound(label));
    }

private:
    struct Node
    {
        Index left = 0;
        Index right = 0;
        int count = 1;
    };

    UnionFind groups_;
    std::vector<T> values_;
    std::vector<Index> roots_;
    std::vector<Node> nodes_;

    Index LowerBound(const T &label) const
    {
        return static_cast<Index>(std::lower_bound(values_.begin(), values_.end(), label) - values_.begin());
    }

    int SizeOf(Index id) const
    {
        return id == 0 ? 0 : nodes_[id - 1].count;
    }

    Index BuildSingleton(Index from, Index to, Index position)
    {
        Node node;
        if (to - from > 1)
        {
            const auto mid = from + (to - from) / 2;
            if (position < mid)
            {
                node.left = BuildSingleton(from, mid, position);
            }
            else
            {
                node.right = BuildSingleton(mid, to, position);
            }
        }
        nodes_.push_back(node);
        return nodes_.size();
    }

    Index MergeTrees(Index first, Index second)
    {
        if (first == 0)
        {
            return second;
        }
        if (second == 0)
        {
            return first;
        }
        auto &left = nodes_[first - 1];
        const auto &right = nodes_[second - 1];
        left.left = MergeTrees(left.left, right.left);
        left.right = MergeTrees(left.right, right.right);
        left.count += right.count;
        return first;
    }

    int Prefix(Index id, Index from, Index to, Index end) const
    {
        if (id == 0 || end <= from)
        {
            return 0;
        }
        const auto &node = nodes_[id - 1];
        if (to <= end)
        {
            return node.count;
        }
        const auto mid = from + (to - from) / 2;
        return Prefix(node.left, from, mid, end) + Prefix(node.right, mid, to, end);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
