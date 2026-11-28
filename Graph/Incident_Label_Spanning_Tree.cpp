// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/incident_label_spanning_tree.md
//
// API一覧
// LabeledSpanningEdge<Label>{from,to,label}: 0始まりの端点とラベルを持つ無向辺。
// incident_label_spanning_tree(required,edges): 全頂点vが required[v] と同じラベルの選択辺に接する全域木。
//   選んだ辺の0始まり入力IDを返す。存在しなければ std::nullopt。多重辺と自己辺可。空グラフなら空列。
//   Label は等値比較可能。O(n+m α(n+1)) 償却時間、O(n+m) 領域。
//   両端を満たす辺で森を作り、満たした頂点から片側の条件を満たす辺を伸ばしてから全域木にする。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <optional>
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
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(i) >= param.size())
            throw std::out_of_range("UnionFind: vertex");
#else
        (void)i;
#endif
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

#ifndef SIRKIT_INCIDENT_LABEL_SPANNING_TREE_DEFINED
#define SIRKIT_INCIDENT_LABEL_SPANNING_TREE_DEFINED

template <class Label>
struct LabeledSpanningEdge
{
    int from, to;
    Label label;
};

template <class Label>
[[nodiscard]] std::optional<std::vector<std::size_t>> incident_label_spanning_tree(
    std::span<const Label> required, std::span<const LabeledSpanningEdge<Label>> edges)
{
    UnionFind forest(required.size());
    const auto n = static_cast<int>(forest.size());
    std::vector<std::size_t> selected;
    std::vector<bool> covered(n);
    struct Candidate
    {
        int endpoint;
        std::size_t edge;
    };
    std::vector<std::vector<Candidate>> choices(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[u, v, label] = edges[id];
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("incident_label_spanning_tree: edge endpoint");
        }
        const bool covers_u = label == required[u], covers_v = label == required[v];
        if (covers_u && covers_v && forest.merge(u, v))
        {
            selected.push_back(id);
            covered[u] = covered[v] = true;
        }
        if (covers_u)
        {
            choices[v].push_back({u, id});
        }
        if (covers_v)
        {
            choices[u].push_back({v, id});
        }
    }

    std::vector<int> pending;
    for (int v = 0; v < n; ++v)
    {
        if (covered[v])
        {
            pending.push_back(v);
        }
    }
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        for (const auto &[endpoint, id] : choices[pending[i]])
        {
            if (!covered[endpoint])
            {
                covered[endpoint] = true;
                pending.push_back(endpoint);
                selected.push_back(id);
                forest.unite(edges[id].from, edges[id].to);
            }
        }
    }
    if (pending.size() != required.size())
    {
        return std::nullopt;
    }

    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        if (forest.merge(edges[i].from, edges[i].to))
        {
            selected.push_back(i);
        }
    }
    if (forest.component_count() > 1)
    {
        return std::nullopt;
    }
    return selected;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
