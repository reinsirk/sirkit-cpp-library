// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Chinese_Postman_Route.cpp
// 参考（奇数次数頂点のペアリング）: https://doi.org/10.1007/BF01580113
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/chinese_postman_route.md
//
// API一覧
// PostmanEdge{from,to,weight}: 0始まりの端点と非負の uint64_t 重みを持つ無向辺。自己辺・多重辺可。
// minimum_postman_route(n, edges, start=-1, finish=-1): 全辺を1回以上通る最小費用ルート。
//   start=-1 は開始点を自動選択。finish=-1 は開始点に戻る。ルートがなければ std::nullopt。
//   PostmanRoute{cost,vertices,edge_ids} は費用、通過頂点列、対応する入力辺ID列。
//   奇数次数とすべき端点を補正した後のペアリング対象が k 頂点。k<=22。
//   O(n+m + k(n+m) log(m+1) + k 2^k) 時間、O((k+1)n + m + 2^k) 領域。
//   kが上限を超えれば length_error、費用が uint64_t を超えれば overflow_error。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EULERIAN_TRAIL_DEFINED
#define SIRKIT_EULERIAN_TRAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<bool Directed>
inline std::size_t EulerianArcCount(std::size_t edges) {
    constexpr std::size_t copies = Directed ? 1 : 2;
    if (edges > std::vector<std::size_t>{}.max_size() / copies)
        throw std::length_error("EulerianTrail: too many edges");
    return edges * copies;
}
}
template<bool Directed = false>
class EulerianTrail {
public:
    struct Trail {
        std::vector<int> vertices;
        std::vector<std::size_t> edges;
        bool circuit() const noexcept { return vertices.empty() || vertices.front() == vertices.back(); }
    };
private:
    int n_ = 0;
    std::vector<std::pair<int, int>> edges_;
public:
    explicit EulerianTrail(int n = 0) : n_(n) {
        if (n < 0) throw std::length_error("EulerianTrail: negative vertex count");
    }
    EulerianTrail(const EulerianTrail&) = default;
    EulerianTrail(EulerianTrail&& other) noexcept { swap(other); }
    EulerianTrail& operator=(EulerianTrail other) noexcept { swap(other); return *this; }
    void swap(EulerianTrail& other) noexcept { std::swap(n_, other.n_); edges_.swap(other.edges_); }
    void reset(int n = 0) { EulerianTrail replacement(n); swap(replacement); }
    int size() const noexcept { return n_; }
    std::size_t edge_count() const noexcept { return edges_.size(); }
    void reserve_edges(std::size_t count) { edges_.reserve(count); }
    std::size_t add_edge(int u, int v) {
        edges_.emplace_back(u, v); return edges_.size() - 1;
    }
    std::pair<int, int> edge(std::size_t id) const {
        return edges_[id];
    }
    std::optional<Trail> find(int start = -1, bool circuit_only = false) const {
        const auto m = edges_.size();
        if (m == 0) {
            Trail result;
            if (n_ != 0) result.vertices.push_back(start == -1 ? 0 : start);
            return result;
        }
        const auto arcs = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::EulerianArcCount<Directed>(m);
        std::vector<std::size_t> degree(n_), incoming;
        if constexpr (Directed) incoming.resize(n_);
        for (auto [u, v] : edges_) {
            ++degree[u];
            if constexpr (Directed) ++incoming[v]; else ++degree[v];
        }
        int preferred = -1, second = -1, any = -1;
        for (int v = 0; v < n_; ++v) {
            if (degree[v] != 0 && any == -1) any = v;
            if constexpr (Directed) {
                if (degree[v] == incoming[v]) continue;
                if (circuit_only) return std::nullopt;
                if (degree[v] > incoming[v] && degree[v] - incoming[v] == 1) {
                    if (preferred != -1) return std::nullopt;
                    preferred = v;
                } else if (incoming[v] > degree[v] && incoming[v] - degree[v] == 1) {
                    if (second != -1) return std::nullopt;
                    second = v;
                } else return std::nullopt;
            } else if ((degree[v] & 1) != 0) {
                if (circuit_only) return std::nullopt;
                if (preferred == -1) preferred = v;
                else if (second == -1) second = v;
                else return std::nullopt;
            }
        }
        if ((preferred == -1) != (second == -1)) return std::nullopt;
        if (start == -1) start = preferred == -1 ? any : preferred;
        else if constexpr (Directed) {
            if ((preferred != -1 && start != preferred) || degree[start] == 0) return std::nullopt;
        } else if ((preferred != -1 && start != preferred && start != second) || degree[start] == 0) return std::nullopt;
        std::vector<std::size_t> begin(static_cast<std::size_t>(n_) + 1);
        for (int v = 0; v < n_; ++v) begin[v + 1] = begin[v] + degree[v];
        std::vector<std::size_t> cursor(begin.begin(), begin.end() - 1), incident(arcs);
        for (std::size_t id = 0; id < m; ++id) {
            const auto [u, v] = edges_[id]; incident[cursor[u]++] = id;
            if constexpr (!Directed) incident[cursor[v]++] = id;
        }
        std::copy_n(begin.begin(), n_, cursor.begin());
        std::vector<unsigned char> used;
        if constexpr (!Directed) used.resize(m);
        std::vector<int> vertices; std::vector<std::size_t> edge_stack;
        vertices.reserve(m + 1); edge_stack.reserve(m); vertices.push_back(start);
        Trail result; result.vertices.reserve(m + 1); result.edges.reserve(m);
        while (!vertices.empty()) {
            const int v = vertices.back();
            if constexpr (!Directed) while (cursor[v] != begin[v + 1] && used[incident[cursor[v]]]) ++cursor[v];
            if (cursor[v] == begin[v + 1]) {
                result.vertices.push_back(v); vertices.pop_back();
                if (!edge_stack.empty()) { result.edges.push_back(edge_stack.back()); edge_stack.pop_back(); }
                continue;
            }
            const auto id = incident[cursor[v]++]; const auto [a, b] = edges_[id];
            if constexpr (!Directed) used[id] = 1;
            vertices.push_back(Directed ? b : (a == v ? b : a)); edge_stack.push_back(id);
        }
        if (result.edges.size() != m) return std::nullopt;
        std::reverse(result.vertices.begin(), result.vertices.end()); std::reverse(result.edges.begin(), result.edges.end());
        return result;
    }
};
using DirectedEulerianTrail = EulerianTrail<true>;
using UndirectedEulerianTrail = EulerianTrail<false>;
#endif

#ifndef SIRKIT_CHINESE_POSTMAN_ROUTE_DEFINED
#define SIRKIT_CHINESE_POSTMAN_ROUTE_DEFINED

struct PostmanEdge
{
    int from, to;
    std::uint64_t weight;
};

struct PostmanRoute
{
    std::uint64_t cost = 0;
    std::vector<int> vertices;
    std::vector<std::size_t> edge_ids;
};

namespace detail
{
inline int other_postman_endpoint(const PostmanEdge &edge, int vertex)
{
    return edge.from == vertex ? edge.to : edge.from;
}

inline std::uint64_t checked_postman_sum(std::uint64_t a, std::uint64_t b)
{
    if (b > std::numeric_limits<std::uint64_t>::max() - a)
    {
        throw std::overflow_error("minimum_postman_route: total cost overflow");
    }
    return a + b;
}

using PostmanAdjacency = std::vector<std::vector<std::size_t>>;

inline std::pair<std::vector<std::optional<std::uint64_t>>, std::vector<std::size_t>> postman_distances(
    const PostmanAdjacency &graph, std::span<const PostmanEdge> edges, int source)
{
    using Entry = std::pair<std::uint64_t, int>;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pending;
    std::vector<std::optional<std::uint64_t>> distance(graph.size());
    std::vector<std::size_t> parent_edge(graph.size());
    distance[source] = 0;
    pending.emplace(0, source);
    while (!pending.empty())
    {
        const auto [prefix, vertex] = pending.top();
        pending.pop();
        if (prefix != distance[vertex].value())
        {
            continue;
        }
        for (const std::size_t id : graph[vertex])
        {
            const auto &edge = edges[id];
            if (edge.weight > std::numeric_limits<std::uint64_t>::max() - prefix)
            {
                continue;
            }
            const int next = other_postman_endpoint(edge, vertex);
            const std::uint64_t next_distance = prefix + edge.weight;
            if (!distance[next] || next_distance < distance[next].value())
            {
                distance[next] = next_distance;
                parent_edge[next] = id;
                pending.emplace(next_distance, next);
            }
        }
    }
    return {std::move(distance), std::move(parent_edge)};
}

inline std::vector<bool> postman_repetitions(
    const PostmanAdjacency &graph, std::span<const PostmanEdge> edges, const std::vector<int> &odd)
{
    std::vector<bool> repeat(edges.size());
    if (odd.empty())
    {
        return repeat;
    }
    const std::size_t k = odd.size();
    std::vector predecessor(k - 1, std::vector<std::size_t>());
    std::vector<std::uint64_t> metric(k * k);
    for (std::size_t from = 0; from + 1 < k; ++from)
    {
        auto [distance, parent_edge] = postman_distances(graph, edges, odd[from]);
        for (std::size_t to = from + 1; to < k; ++to)
        {
            metric[from * k + to] = distance[odd[to]].value();
        }
        predecessor[from] = std::move(parent_edge);
    }

    const auto all = (std::uint32_t{1} << k) - 1;
    std::vector<std::uint64_t> best(all + 1);
    std::vector<std::uint8_t> partner(all + 1);
    const auto solve = [&](auto &&self, std::uint32_t mask) -> std::uint64_t
    {
        if (mask == 0 || partner[mask] != 0)
        {
            return best[mask];
        }
        const auto first = std::countr_zero(mask);
        const std::uint32_t others = mask & (mask - 1);
        for (std::uint32_t available = others; available != 0; available &= available - 1)
        {
            const auto second = std::countr_zero(available);
            const auto suffix = self(self, others ^ (std::uint32_t{1} << second));
            const auto length = metric[first * k + second];
            if (length <= std::numeric_limits<std::uint64_t>::max() - suffix)
            {
                const auto candidate = length + suffix;
                if (partner[mask] == 0 || candidate < best[mask])
                {
                    best[mask] = candidate;
                    partner[mask] = static_cast<std::uint8_t>(second + 1);
                }
            }
        }
        return best[mask];
    };

    (void)solve(solve, all);
    for (auto remaining = all; remaining != 0;)
    {
        const auto u = std::countr_zero(remaining);
        const auto v = partner[remaining] - 1;
        // 両方の最短路に現れる辺を2本ずつ除いても、追加辺の次数の偶奇は維持される。
        for (int current = odd[v]; current != odd[u];)
        {
            const auto edge = predecessor[u][current];
            repeat[edge] = !repeat[edge];
            current = other_postman_endpoint(edges[edge], current);
        }
        remaining &= remaining - 1;
        remaining ^= std::uint32_t{1} << v;
    }
    return repeat;
}
} // namespace detail

[[nodiscard]] inline std::optional<PostmanRoute> minimum_postman_route(
    int n, std::span<const PostmanEdge> edges, int start = -1, int finish = -1)
{
    if (n < 0)
    {
        throw std::length_error("minimum_postman_route: negative vertex count");
    }
    if (start < -1 || start >= n || finish < -1 || finish >= n)
    {
        throw std::out_of_range("minimum_postman_route: terminal");
    }
    if (edges.empty())
    {
        if (n == 0)
        {
            return PostmanRoute{};
        }
        const int first = start == -1 ? 0 : start;
        if (finish != -1 && finish != first)
        {
            return std::nullopt;
        }
        return PostmanRoute{0, {first}, {}};
    }

    detail::PostmanAdjacency graph(n);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto [u, v, weight] = edges[i];
        if (static_cast<std::size_t>(u) >= graph.size() || static_cast<std::size_t>(v) >= graph.size())
        {
            throw std::out_of_range("minimum_postman_route: edge endpoint");
        }
        graph[u].push_back(i);
        graph[v].push_back(i);
    }
    if (start == -1)
    {
        start = edges.front().from;
    }
    if (finish == -1)
    {
        finish = start;
    }

    std::vector<bool> seen(n);
    std::vector<int> pending{start};
    seen[start] = true;
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        const int vertex = pending[i];
        for (const auto edge : graph[vertex])
        {
            const int next = detail::other_postman_endpoint(edges[edge], vertex);
            if (!seen[next])
            {
                seen[next] = true;
                pending.push_back(next);
            }
        }
    }
    if (!seen[finish] || std::ranges::any_of(edges, [&](const PostmanEdge &edge)
                                             {
                                                 return !seen[edge.from];
                                             }))
    {
        return std::nullopt;
    }

    std::vector<int> odd;
    for (int vertex = 0; vertex < n; ++vertex)
    {
        const bool required_odd = (vertex == start) != (vertex == finish);
        if ((graph[vertex].size() % 2 != 0) != required_odd)
        {
            odd.push_back(vertex);
        }
    }
    constexpr std::size_t kMaximumOddVertices = 22;
    if (odd.size() > kMaximumOddVertices)
    {
        throw std::length_error("minimum_postman_route: too many odd vertices");
    }
    std::uint64_t cost = 0;
    for (const auto &edge : edges)
    {
        cost = detail::checked_postman_sum(cost, edge.weight);
    }
    const auto repeat = detail::postman_repetitions(graph, edges, odd);

    UndirectedEulerianTrail trail(n);
    trail.reserve_edges(edges.size());
    std::vector<std::size_t> original;
    original.reserve(edges.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        trail.add_edge(edges[i].from, edges[i].to);
        original.push_back(i);
        if (repeat[i])
        {
            trail.add_edge(edges[i].from, edges[i].to);
            original.push_back(i);
            cost = detail::checked_postman_sum(cost, edges[i].weight);
        }
    }
    auto route = trail.find(start, start == finish).value();
    for (auto &i : route.edges)
    {
        i = original[i];
    }
    return PostmanRoute{cost, std::move(route.vertices), std::move(route.edges)};
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
