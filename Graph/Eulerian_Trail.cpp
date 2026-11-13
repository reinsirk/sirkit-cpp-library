// SPDX-License-Identifier: CC0-1.0
// 参考（オイラー路の条件）: https://ei1333.github.io/library/graph/others/eulerian-trail.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/eulerian_trail.md
//
// API一覧
// EulerianTrail<Directed=false>(n=0): n頂点。有向/無向を型で選ぶ。自己辺・多重辺可。準備O(1)。
// DirectedEulerianTrail/UndirectedEulerianTrail: 上記の有向/無向の別名。
// add_edge(u,v): 辺を追加しsize_tのIDを返す。償却O(1)。reserve_edges(m): m辺予約。
// size()/edge_count()/edge(id): 頂点数/辺数/追加時の端点pair。O(1)。
// find(start=-1,circuit_only=false): 全辺を一度ずつ通るTrailのoptional。存在しなければnullopt。O(n+m)時間・領域。
// Trail.verticesは通過頂点列、Trail.edgesは対応する辺ID列。circuit()は閉路か。無向辺の向きは頂点列で分かる。
// start=-1は自動選択。指定開始点が次数条件に合わない場合もnullopt。circuit_only=trueは閉路だけを許す。
// 0辺なら指定開始点（省略時0）だけを返す。0頂点0辺なら空列のTrailを返す。
// 辺を持つ成分が複数ならnullopt。孤立頂点は路に含めない。findはグラフを変更せず繰り返し呼べる。
// reset(n=0)/swap(other): 空のn頂点へ/O(1)交換。コピーは独立所有、ムーブ元は空。
// 頂点・辺IDの入力検査はSKIP_RUNTIME_CHECKSで除く。次数条件と全辺の消費判定は維持する。

#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_EULERIAN_TRAIL_DEFINED
#define SIRKIT_EULERIAN_TRAIL_DEFINED
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
        constexpr std::size_t copies = Directed ? 1 : 2;
        if (m > std::vector<std::size_t>().max_size() / copies)
            throw std::length_error("EulerianTrail: too many edges");
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
        std::vector<std::size_t> cursor(begin.begin(), begin.end() - 1), incident(m * copies);
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
