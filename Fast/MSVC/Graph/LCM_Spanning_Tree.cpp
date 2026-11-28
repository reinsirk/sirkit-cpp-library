// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/LCM_Spanning_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lcm_spanning_tree.md
//
// API一覧
// LcmSpanningTreeEdge{from,to,weight}: 頂点番号は0始まり、辺の重みはuint64_t。
// minimum_lcm_spanning_tree(labels): 正のuint32_tラベルを持つ完全グラフの最小全域木の辺を返す。
// 辺の重みは両端のラベルの最小公倍数。空と頂点1個は空の辺列。ラベル0はinvalid_argument。
// n<=INT_MAX。全ラベルを最大公約数で割った最大値Aに対しO(n log(U+1)+A log²(A+1))時間、
// O(n+A log(A+1))領域。Uは最大ラベル。最小ラベルが他を割り切る場合はO(n log(U+1))時間。
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
#include <cstdint>
#include <span>


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

#ifndef SIRKIT_LCM_SPANNING_TREE_DEFINED
#define SIRKIT_LCM_SPANNING_TREE_DEFINED

struct LcmSpanningTreeEdge
{
    int from;
    int to;
    std::uint64_t weight;
};

[[nodiscard]] inline std::vector<LcmSpanningTreeEdge> minimum_lcm_spanning_tree(std::span<const std::uint32_t> labels)
{
    if (labels.empty())
    {
        return {};
    }
    const auto [minimum, maximum] = std::ranges::minmax_element(labels);
    if (*minimum == 0)
    {
        throw std::invalid_argument("minimum_lcm_spanning_tree: zero label");
    }
    UnionFind components(labels.size());
    std::uint32_t scale = *minimum;
    for (auto label : labels)
    {
        scale = std::gcd(scale, label);
        if (scale == 1)
        {
            break;
        }
    }
    std::vector<LcmSpanningTreeEdge> result;
    result.reserve(labels.size() - 1);
    if (scale == *minimum)
    {
        const auto center = minimum - labels.begin();
        for (std::size_t v = 0; v < labels.size(); ++v)
        {
            if (v != static_cast<std::size_t>(center))
            {
                result.push_back({static_cast<int>(center), static_cast<int>(v), labels[v]});
            }
        }
        return result;
    }

    const std::size_t bound = *maximum / scale;
    std::vector<int> representative(bound + 1, -1);
    for (std::size_t v = 0; v < labels.size(); ++v)
    {
        const auto label = labels[v];
        auto &first = representative[label / scale];
        if (first == -1)
        {
            first = static_cast<int>(v);
        }
        else
        {
            components.unite(first, static_cast<int>(v));
            result.push_back({first, static_cast<int>(v), label});
        }
    }

    std::vector<LcmSpanningTreeEdge> candidates;
    for (std::size_t divisor = 1; divisor <= bound; ++divisor)
    {
        std::size_t center = 0;
        for (auto label = divisor; label <= bound; label += divisor)
        {
            const auto vertex = representative[label];
            if (vertex == -1)
            {
                continue;
            }
            if (center != 0)
            {
                // 最大公約数がdivisorの元の辺は、その倍数で最小のラベルを経由して置き換えられる。
                const auto weight = std::uint64_t{center / divisor} * label * scale;
                candidates.push_back({representative[center], vertex, weight});
            }
            else
            {
                center = label;
            }
        }
    }
    std::ranges::sort(candidates, {}, &LcmSpanningTreeEdge::weight);
    for (auto edge = candidates.begin(); result.size() + 1 < labels.size(); ++edge)
    {
        if (components.merge(edge->from, edge->to))
        {
            result.push_back(*edge); // 過大評価した辺は、同じ両端を結ぶ軽い経路が先にできるため採用されない。
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
