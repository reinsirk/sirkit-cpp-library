// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Bottleneck_Pair_Histogram.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bottleneck_pair_histogram.md
//
// API一覧
// BottleneckEdge{from,to,weight}: 0始まりの無向辺。重みは符号付き64ビット。自己辺・多重辺可。
// bottleneck_pair_histogram(n,edges): 同じ成分に属する相異なる頂点の順序なしペアを数える。
//   BottleneckPairBucket{weight,minimax_pairs,maximin_pairs} が重みの昇順に並ぶ。
//   minimax はパスの最大辺重みの最小値、maximin はパスの最小辺重みの最大値。
//   入力の各重みにつき1件。該当するペアがなければ個数0。n<=INT_MAX。
//   時間 O(n+m log(m+1))、領域 O(n+m)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
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
    // 単一成分の連続処理を分離し、通常の成分列挙のコード生成を保つ。
    void list_single_component(std::vector<int>& vertices) {
        const int root = leader_unchecked(0);
        const int negative_size = param[root];
        std::fill(param.begin(), param.end(), root);
        param[root] = negative_size;
        std::iota(vertices.begin(), vertices.end(), 0);
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
        a = leader_unchecked(a); b = leader_unchecked(b);
        if (a == b) return false;
        if (param[a] > param[b]) std::swap(a, b);
        param[a] += param[b]; param[b] = a; --components_;
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
            result.emplace_back(param.size());
            list_single_component(result[0]);
            return result;
        }
        std::vector<int> leaders(param.size()), counts(param.size(), 0);
        for (int i = 0; i < size(); ++i) ++counts[leaders[i] = leader_unchecked(i)];
        for (int i = 0; i < size(); ++i) if (counts[i]) {
            const int count = counts[i];
            counts[i] = static_cast<int>(result.size());
            result.emplace_back(); result.back().reserve(count);
        }
        for (int i = 0; i < size(); ++i) result[counts[leaders[i]]].push_back(i);
        return result;
    }
};
#endif

#ifndef SIRKIT_BOTTLENECK_PAIR_HISTOGRAM_DEFINED
#define SIRKIT_BOTTLENECK_PAIR_HISTOGRAM_DEFINED

struct BottleneckEdge
{
    int from;
    int to;
    std::int64_t weight;
};

struct BottleneckPairBucket
{
    std::int64_t weight;
    std::uint64_t minimax_pairs = 0;
    std::uint64_t maximin_pairs = 0;
};

inline std::vector<BottleneckPairBucket> bottleneck_pair_histogram(
    std::size_t n, std::span<const BottleneckEdge> edges)
{
    UnionFind components(n);
    std::vector<BottleneckEdge> sorted(edges.begin(), edges.end());
    std::ranges::sort(sorted, {}, &BottleneckEdge::weight);

    std::vector<BottleneckPairBucket> result;
    std::vector<std::size_t> boundary{0};
    for (std::size_t i = 0; i < sorted.size();)
    {
        const auto weight = sorted[i].weight;
        result.push_back({weight});
        do
        {
            ++i;
        } while (i < sorted.size() && sorted[i].weight == weight);
        boundary.push_back(i);
    }

    auto merge_group = [&](std::size_t group)
    {
        std::uint64_t added = 0;
        for (std::size_t i = boundary[group]; i < boundary[group + 1]; ++i)
        {
            const int u = components.root(sorted[i].from);
            const int v = components.root(sorted[i].to);
            if (u != v)
            {
                added += static_cast<std::uint64_t>(components.size(u)) * components.size(v);
                components.merge(u, v);
            }
        }
        return added;
    };
    for (std::size_t group = 0; group < result.size(); ++group)
    {
        result[group].minimax_pairs = merge_group(group);
    }
    components.reset();
    for (std::size_t group = result.size(); group != 0;)
    {
        --group;
        result[group].maximin_pairs = merge_group(group);
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
