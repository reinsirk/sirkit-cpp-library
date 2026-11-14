// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Offline_Dynamic_Connectivity.cpp
// 参考: https://nyaannyaan.github.io/library/graph/offline-dynamic-connectivity.hpp
// 区間分割とrollback Union-Findの数学的構成を参考に独立実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/offline_dynamic_connectivity.md
// 頂点番号・有効辺IDの検査はSKIP_RUNTIME_CHECKSで除外。内部の容量・添字上限保護は維持。
// API一覧
// OfflineDynamicConnectivity(n=0) / (span<const int64_t> weights): n頂点、初期値0/指定値。O(n)。
// 頂点は0..n-1。自己辺・多重辺可。辺IDは発行元（独立コピーを含む）の履歴内でのみ使用する。
// add_edge(u,v): 有効な辺を追加しIDを返す。remove_edge(id): 有効なその辺を削除。償却O(1)。
// add_vertex(v,delta): 以降の照会の頂点値に加算。償却O(1)。
// query_same(u,v) / query_size(v) / query_component_count() / query_component_sum(v):
//   現時点の照会を記録し回答添字を返す。償却O(1)。sameの回答は0/1。
// solve() const: 照会順のvector<int64_t>。記録は消さず、追記・再実行可。空照会は空vector。
//   E=追加辺数、A=頂点加算数、Q=照会数としてO(n+(E+A+Q)log(Q+1)log(n+1))時間、
//   O(n+(E+A)log(Q+1)+Q)作業領域。rollbackは内部処理で、外部への状態巻戻しAPIではない。
// size()/empty(), edge_count()/query_count()/addition_count(): O(1)。edge_countは削除済みも含む。
// reserve_edges/additions/queries(cap): 対応する記録領域を予約。O(現在の記録数)。
// clear_history(): 初期頂点値を維持し全記録を消去。reset(n)/reset(weights): 初期化。clear(): 0頂点。
// copyは全記録を独立所有。move元は0頂点で再利用可。swapはO(1)。
// 確保失敗時は内容を維持。solveは常に元オブジェクトを維持。span引数との自己参照可。
// 頂点値と成分和は、内部の加算順を含め全てint64_tに収まること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CONNECTIVITY_ROLLBACK_STATE_DEFINED
#define SIRKIT_CONNECTIVITY_ROLLBACK_STATE_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
class ConnectivityRollbackState {
    struct Change { int root, child, root_size, child_size; std::int64_t root_sum; };
    std::vector<int> parent_;
    std::vector<std::int64_t> sum_;
    std::vector<Change> history_;
    int components_;
public:
    explicit ConnectivityRollbackState(std::span<const std::int64_t> weights, std::size_t reserve)
        : parent_(weights.size(), -1), sum_(weights.begin(), weights.end()), components_(static_cast<int>(weights.size())) {
        history_.reserve(reserve);
    }
    int leader(int v) const noexcept { while (parent_[v] >= 0) v = parent_[v]; return v; }
    bool same(int u, int v) const noexcept { return leader(u) == leader(v); }
    int size(int v) const noexcept { return -parent_[leader(v)]; }
    std::int64_t sum(int v) const noexcept { return sum_[leader(v)]; }
    int components() const noexcept { return components_; }
    std::size_t snapshot() const noexcept { return history_.size(); }
    void merge(int u, int v) {
        u = leader(u); v = leader(v);
        if (u == v) return;
        if (parent_[u] > parent_[v]) std::swap(u, v);
        history_.push_back({u, v, parent_[u], parent_[v], sum_[u]});
        parent_[u] += parent_[v]; parent_[v] = u; sum_[u] += sum_[v]; --components_;
    }
    void add(int v, std::int64_t delta) {
        v = leader(v);
        history_.push_back({v, -1, 0, 0, sum_[v]});
        sum_[v] += delta;
    }
    void rollback(std::size_t snapshot) noexcept {
        while (history_.size() != snapshot) {
            const auto c = history_.back(); history_.pop_back(); sum_[c.root] = c.root_sum;
            if (c.child != -1) { parent_[c.root] = c.root_size; parent_[c.child] = c.child_size; ++components_; }
        }
    }
};
}
#endif

#ifndef SIRKIT_OFFLINE_DYNAMIC_CONNECTIVITY_DEFINED
#define SIRKIT_OFFLINE_DYNAMIC_CONNECTIVITY_DEFINED
class OfflineDynamicConnectivity {
    static constexpr std::size_t ongoing = std::numeric_limits<std::size_t>::max();
    struct Edge { int u, v; std::size_t first, last; };
    struct Addition { int v; std::int64_t delta; std::size_t first; };
    enum class Kind { same, size, count, sum };
    struct Query { Kind kind; int u, v; };
    std::vector<std::int64_t> initial_;
    std::vector<Edge> edges_;
    std::vector<Addition> additions_;
    std::vector<Query> queries_;
    void check_vertex(int v) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || static_cast<std::size_t>(v) >= initial_.size()) throw std::out_of_range("OfflineDynamicConnectivity vertex");
#else
        (void)v;
#endif
    }
    std::size_t query(Kind kind, int u, int v = 0) {
        const auto id = queries_.size(); queries_.push_back({kind, u, v}); return id;
    }
    template<class F> void intervals(std::size_t base, F&& f) const {
        const auto q = queries_.size();
        const auto visit = [&](std::size_t id, std::size_t first, std::size_t last) {
            for (auto l = first + base, r = last + base; l < r; l >>= 1, r >>= 1) {
                if (l & 1) f(l++, id);
                if (r & 1) f(--r, id);
            }
        };
        for (std::size_t i = 0; i < edges_.size(); ++i) {
            const auto& e = edges_[i]; visit(i, e.first, e.last == ongoing ? q : e.last);
        }
        for (std::size_t i = 0; i < additions_.size(); ++i) visit(edges_.size() + i, additions_[i].first, q);
    }
public:
    explicit OfflineDynamicConnectivity(int n = 0) {
        if (n < 0) throw std::length_error("OfflineDynamicConnectivity size");
        initial_.resize(static_cast<std::size_t>(n));
    }
    explicit OfflineDynamicConnectivity(std::span<const std::int64_t> weights) {
        if (weights.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) throw std::length_error("OfflineDynamicConnectivity size");
        initial_.assign(weights.begin(), weights.end());
    }
    OfflineDynamicConnectivity(const OfflineDynamicConnectivity&) = default;
    OfflineDynamicConnectivity& operator=(const OfflineDynamicConnectivity& other) {
        if (this != &other) { OfflineDynamicConnectivity copy(other); swap(copy); } return *this;
    }
    OfflineDynamicConnectivity(OfflineDynamicConnectivity&& other) noexcept { swap(other); }
    OfflineDynamicConnectivity& operator=(OfflineDynamicConnectivity&& other) noexcept {
        if (this != &other) { clear(); swap(other); } return *this;
    }
    void swap(OfflineDynamicConnectivity& other) noexcept {
        initial_.swap(other.initial_); edges_.swap(other.edges_); additions_.swap(other.additions_); queries_.swap(other.queries_);
    }
    friend void swap(OfflineDynamicConnectivity& a, OfflineDynamicConnectivity& b) noexcept { a.swap(b); }
    int size() const noexcept { return static_cast<int>(initial_.size()); }
    bool empty() const noexcept { return initial_.empty(); }
    std::size_t edge_count() const noexcept { return edges_.size(); }
    std::size_t addition_count() const noexcept { return additions_.size(); }
    std::size_t query_count() const noexcept { return queries_.size(); }
    void reserve_edges(std::size_t cap) { edges_.reserve(cap); }
    void reserve_additions(std::size_t cap) { additions_.reserve(cap); }
    void reserve_queries(std::size_t cap) { queries_.reserve(cap); }
    std::size_t add_edge(int u, int v) {
        check_vertex(u); check_vertex(v);
        const auto id = edges_.size(); edges_.push_back({u, v, queries_.size(), ongoing}); return id;
    }
    void remove_edge(std::size_t id) {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= edges_.size() || edges_[id].last != ongoing) throw std::out_of_range("OfflineDynamicConnectivity active edge");
#endif
        edges_[id].last = queries_.size();
    }
    void add_vertex(int v, std::int64_t delta) { check_vertex(v); additions_.push_back({v, delta, queries_.size()}); }
    std::size_t query_same(int u, int v) { check_vertex(u); check_vertex(v); return query(Kind::same, u, v); }
    std::size_t query_size(int v) { check_vertex(v); return query(Kind::size, v); }
    std::size_t query_component_count() { return query(Kind::count, 0); }
    std::size_t query_component_sum(int v) { check_vertex(v); return query(Kind::sum, v); }
    std::vector<std::int64_t> solve() const {
        const auto q = queries_.size(); if (!q) return {};
        const auto limit = std::vector<std::size_t>().max_size();
        std::size_t base = 1;
        while (base < q) {
            if (base > (limit - 1) / 4) throw std::length_error("OfflineDynamicConnectivity timeline");
            base *= 2;
        }
        if (base > (limit - 1) / 2 || additions_.size() > ongoing - edges_.size()) throw std::length_error("OfflineDynamicConnectivity capacity");
        const auto nodes = base * 2;
        std::vector<std::size_t> offsets(nodes + 1);
        intervals(base, [&](std::size_t node, std::size_t) {
            if (offsets[node + 1] == limit) throw std::length_error("OfflineDynamicConnectivity intervals");
            ++offsets[node + 1];
        });
        for (std::size_t i = 1; i <= nodes; ++i) {
            if (offsets[i] > limit - offsets[i - 1]) throw std::length_error("OfflineDynamicConnectivity intervals");
            offsets[i] += offsets[i - 1];
        }
        std::vector<std::size_t> ids(offsets.back()), cursor(offsets.begin(), offsets.end() - 1);
        intervals(base, [&](std::size_t node, std::size_t id) { ids[cursor[node]++] = id; });
        const auto merge_bound = std::min(edges_.size(), initial_.empty() ? std::size_t{0} : initial_.size() - 1);
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ConnectivityRollbackState uf(initial_, merge_bound + additions_.size());
        struct Frame { std::size_t node, snapshot; bool exit; };
        std::vector<Frame> stack; stack.reserve(2 * std::bit_width(base) + 1); stack.push_back({1, 0, false});
        std::vector<std::int64_t> answers(q);
        while (!stack.empty()) {
            const auto frame = stack.back(); stack.pop_back();
            if (frame.exit) { uf.rollback(frame.snapshot); continue; }
            const auto state = uf.snapshot();
            for (auto i = offsets[frame.node]; i < offsets[frame.node + 1]; ++i) {
                const auto id = ids[i];
                if (id < edges_.size()) { const auto& e = edges_[id]; uf.merge(e.u, e.v); }
                else { const auto& a = additions_[id - edges_.size()]; uf.add(a.v, a.delta); }
            }
            if (frame.node >= base) {
                const auto index = frame.node - base;
                if (index < q) {
                    const auto& x = queries_[index];
                    switch (x.kind) {
                    case Kind::same: answers[index] = uf.same(x.u, x.v); break;
                    case Kind::size: answers[index] = uf.size(x.u); break;
                    case Kind::count: answers[index] = uf.components(); break;
                    case Kind::sum: answers[index] = uf.sum(x.u); break;
                    }
                }
                uf.rollback(state);
            } else {
                stack.push_back({frame.node, state, true});
                stack.push_back({frame.node * 2 + 1, 0, false}); stack.push_back({frame.node * 2, 0, false});
            }
        }
        return answers;
    }
    void clear_history() noexcept { edges_.clear(); additions_.clear(); queries_.clear(); }
    void reset(int n = 0) { OfflineDynamicConnectivity next(n); swap(next); }
    void reset(std::span<const std::int64_t> weights) { OfflineDynamicConnectivity next(weights); swap(next); }
    void clear() noexcept { initial_.clear(); clear_history(); }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
