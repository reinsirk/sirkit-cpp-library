// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/eulerian_trail_decomposition.md
//
// API一覧
// minimum_eulerian_trail_decomposition<Directed=false>(n, edges):
//   全辺を一度ずつ通る小道のうち、本数が最小となる分解を返す。時間・領域 O(n+m)。
//   頂点は 0 以上 n 未満。edges[i] が辺 ID i。自己辺・多重辺可。
//   Directed=true で有向、既定は無向。辺を含まない成分は結果に含めない。
//   各 EulerianTrail<Directed>::Trail の vertices は頂点列、edges は辺 ID 列。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
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
#ifndef SKIP_RUNTIME_CHECKS
    void vertex(int v) const { if (v < 0 || v >= n_) throw std::out_of_range("EulerianTrail: vertex out of range"); }
#endif
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
#ifndef SKIP_RUNTIME_CHECKS
        vertex(u); vertex(v);
#endif
        edges_.emplace_back(u, v); return edges_.size() - 1;
    }
    std::pair<int, int> edge(std::size_t id) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= edges_.size()) throw std::out_of_range("EulerianTrail: edge out of range");
#endif
        return edges_[id];
    }
    std::optional<Trail> find(int start = -1, bool circuit_only = false) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (start != -1) vertex(start);
#endif
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

#ifndef SIRKIT_EULERIAN_TRAIL_DECOMPOSITION_DEFINED
#define SIRKIT_EULERIAN_TRAIL_DECOMPOSITION_DEFINED

template <bool Directed = false>
std::vector<typename EulerianTrail<Directed>::Trail> minimum_eulerian_trail_decomposition(
    int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0 || n == std::numeric_limits<int>::max())
    {
        throw std::length_error("minimum_eulerian_trail_decomposition: invalid vertex count");
    }
    if (edges.empty())
    {
        return {};
    }

    const int hub = n;
    EulerianTrail<Directed> graph(n + 1);
    graph.reserve_edges(edges.size());
    std::vector<std::size_t> begin(static_cast<std::size_t>(n) + 1);
    std::vector<std::ptrdiff_t> balance;
    if constexpr (Directed)
    {
        balance.resize(n);
    }
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("minimum_eulerian_trail_decomposition: invalid vertex");
        }
#endif
        graph.add_edge(u, v);
        ++begin[u + 1];
        ++begin[v + 1];
        if constexpr (Directed)
        {
            ++balance[u];
            --balance[v];
        }
    }
    for (int u = 0; u < n; ++u)
    {
        begin[u + 1] += begin[u];
    }

    std::vector<int> neighbors(begin.back());
    {
        std::vector<std::size_t> next(begin.begin(), begin.end() - 1);
        for (const auto &[u, v] : edges)
        {
            neighbors[next[u]++] = v;
            neighbors[next[v]++] = u;
        }
    }

    std::vector<unsigned char> visited(n);
    std::vector<int> stack;
    for (int start = 0; start < n; ++start)
    {
        if (visited[start] || begin[start] == begin[start + 1])
        {
            continue;
        }
        bool has_hub_edge = false;
        visited[start] = 1;
        stack.push_back(start);
        while (!stack.empty())
        {
            const int u = stack.back();
            stack.pop_back();
            if constexpr (Directed)
            {
                if (balance[u] != 0)
                {
                    has_hub_edge = true;
                }
                for (; balance[u] > 0; --balance[u])
                {
                    graph.add_edge(hub, u);
                }
                for (; balance[u] < 0; ++balance[u])
                {
                    graph.add_edge(u, hub);
                }
            }
            else
            {
                if ((begin[u + 1] - begin[u]) % 2 != 0)
                {
                    graph.add_edge(hub, u);
                    has_hub_edge = true;
                }
            }
            for (std::size_t i = begin[u]; i < begin[u + 1]; ++i)
            {
                const int v = neighbors[i];
                if (!visited[v])
                {
                    visited[v] = 1;
                    stack.push_back(v);
                }
            }
        }
        if (!has_hub_edge)
        {
            graph.add_edge(hub, start);
            graph.add_edge(start, hub);
        }
    }

    // 仮の頂点を始点とする閉路を取り出し、仮の頂点に戻るたびに小道を区切る。
    const auto circuit = graph.find(hub, true).value();
    std::vector<typename EulerianTrail<Directed>::Trail> trails;
    for (std::size_t i = 0; i < circuit.edges.size(); ++i)
    {
        const int u = circuit.vertices[i];
        const int v = circuit.vertices[i + 1];
        if (u == hub)
        {
            trails.emplace_back();
            trails.back().vertices.push_back(v);
        }
        else if (v != hub)
        {
            trails.back().vertices.push_back(v);
            trails.back().edges.push_back(circuit.edges[i]);
        }
    }
    return trails;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
