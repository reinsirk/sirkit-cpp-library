// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Incremental_Bridges.cpp
// 参考（橋でつないだ成分を保つ考え方）: https://doi.org/10.1007/BF01758773
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/incremental_bridges.md
//
// API一覧
// IncrementalBridges(n): 0始まりの n 頂点、辺なし。有向辺の追加や削除は扱わない。
// add_edge(u,v): 無向辺を追加し入力順の辺IDを返す。自己辺・多重辺可。
// is_bridge(id)/bridge_count(): 現時点で橋か/橋の本数。O(1)。
// size()/edge_count()/component_count()/block_count(): 頂点数/辺数/連結成分数/2辺連結成分数。O(1)。
// same_component(u,v)/same_two_edge_component(u,v): 連結か/任意の辺を1本除いても連結か。償却 O(α(n+1))。
// reserve_edges(m): 辺の判定用領域を m 本分予約できる。
// m 回追加と q 回の成分クエリで O((m+q+n log(n+1)) α(n+1)) 時間、O(n+m) 領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
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
#if defined(__clang__)
    // 単一成分の連続処理を分離し、通常の成分列挙のコード生成を保つ。
    void list_single_component(std::vector<int>& vertices) {
        const int root = leader_unchecked(0);
        const int negative_size = param[root];
        std::fill(param.begin(), param.end(), root);
        param[root] = negative_size;
        std::iota(vertices.begin(), vertices.end(), 0);
    }
#endif
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
#if defined(__GNUC__) && !defined(__clang__)
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
#else
        a = leader_unchecked(a); b = leader_unchecked(b);
        if (a == b) return false;
        if (param[a] > param[b]) std::swap(a, b);
        param[a] += param[b]; param[b] = a; --components_;
#endif
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
#if defined(__clang__)
            result.emplace_back(param.size());
            list_single_component(result[0]);
#else
            const int root = leader_unchecked(0);
            result.emplace_back(param.size());
            const int negative_size = param[root];
            std::fill(param.begin(), param.end(), root);
            param[root] = negative_size;
            std::iota(result[0].begin(), result[0].end(), 0);
#endif
            return result;
        }
#if defined(__clang__)
        std::vector<int> leaders(param.size()), counts(param.size(), 0);
        for (int i = 0; i < size(); ++i) ++counts[leaders[i] = leader_unchecked(i)];
        for (int i = 0; i < size(); ++i) if (counts[i]) {
            const int count = counts[i];
            counts[i] = static_cast<int>(result.size());
            result.emplace_back(); result.back().reserve(count);
        }
        for (int i = 0; i < size(); ++i) result[counts[leaders[i]]].push_back(i);
#else
        std::vector<int> slot(param.size());
        for (int i = 0; i < size(); ++i) if (param[i] < 0) {
            slot[i] = static_cast<int>(result.size());
            result.emplace_back(); result.back().reserve(static_cast<std::size_t>(-param[i]));
        }
        for (int i = 0; i < size(); ++i) result[slot[leader_unchecked(i)]].push_back(i);
#endif
        return result;
    }
};
#endif

#ifndef SIRKIT_INCREMENTAL_BRIDGES_DEFINED
#define SIRKIT_INCREMENTAL_BRIDGES_DEFINED

class IncrementalBridges
{
public:
    explicit IncrementalBridges(std::size_t n)
        : connected_(n), blocks_(n), parent_(n, -1), parent_edge_(n), visited_(n)
    {
    }

    int size() const noexcept
    {
        return connected_.size();
    }

    std::size_t edge_count() const noexcept
    {
        return bridge_flags_.size();
    }

    int bridge_count() const noexcept
    {
        return block_count() - component_count();
    }

    int component_count() const noexcept
    {
        return connected_.component_count();
    }

    int block_count() const noexcept
    {
        return blocks_.component_count();
    }

    bool same_component(int u, int v)
    {
        return connected_.same(u, v);
    }

    bool same_two_edge_component(int u, int v)
    {
        return blocks_.same(u, v);
    }

    void reserve_edges(std::size_t count)
    {
        bridge_flags_.reserve(count);
    }

    bool is_bridge(std::size_t id) const
    {
        return bridge_flags_[id] != 0;
    }

    std::size_t add_edge(int u, int v)
    {
        u = blocks_.leader(u);
        v = blocks_.leader(v);
        const auto id = edge_count();
        if (u == v)
        {
            bridge_flags_.push_back(0);
            return id;
        }
        if (connected_.same(u, v))
        {
            for (auto &path : climbs_)
            {
                path.reserve(static_cast<std::size_t>(size()));
            }
            bridge_flags_.push_back(0);
            merge_cycle(u, v, id + 1);
            return id;
        }
        bridge_flags_.push_back(1);
        if (connected_.size(u) > connected_.size(v))
        {
            std::swap(u, v);
        }
        reroot_smaller_component(u);
        parent_[u] = v;
        parent_edge_[u] = id;
        connected_.merge(u, v);
        return id;
    }

private:
    int forest_parent(int node)
    {
        const int parent = parent_[node];
        return parent == -1 ? -1 : blocks_.leader(parent);
    }

    void reroot_smaller_component(int node)
    {
        int previous = -1;
        std::size_t previous_edge = 0;
        while (node != -1)
        {
            const int next = forest_parent(node);
            parent_[node] = previous;
            std::swap(parent_edge_[node], previous_edge);
            previous = node;
            node = next;
        }
    }

    void merge_cycle(int u, int v, std::size_t stamp)
    {
        climbs_[0].clear();
        climbs_[1].clear();
        int ancestor = -1;
        auto advance = [&](int &node, std::vector<int> &chain)
        {
            if (node == -1)
            {
                return;
            }
            if (visited_[node] == stamp)
            {
                ancestor = node;
                return;
            }
            visited_[node] = stamp;
            chain.push_back(node);
            node = forest_parent(node);
        };
        while (ancestor == -1)
        {
            advance(u, climbs_[0]);
            if (ancestor == -1)
            {
                advance(v, climbs_[1]);
            }
        }

        // 共通祖先の外側につながる橋を、併合後の代表へ引き継ぐ。
        const int outside = parent_[ancestor];
        const auto outside_edge = parent_edge_[ancestor];
        for (const auto &chain : climbs_)
        {
            for (const int node : chain)
            {
                if (node == ancestor)
                {
                    break;
                }
                bridge_flags_[parent_edge_[node]] = 0;
                blocks_.merge(ancestor, node);
            }
        }
        const int leader = blocks_.leader(ancestor);
        parent_[leader] = outside;
        parent_edge_[leader] = outside_edge;
    }

    UnionFind connected_;
    UnionFind blocks_;
    std::vector<int> parent_;
    std::vector<std::size_t> parent_edge_;
    std::vector<std::size_t> visited_;
    std::vector<unsigned char> bridge_flags_;
    std::array<std::vector<int>, 2> climbs_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
