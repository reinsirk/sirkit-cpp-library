// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Functional_Graph.cpp
// 参考: https://nyaannyaan.github.io/library/graph/functional-graph.hpp
// https://atcoder.jp/contests/abc367/tasks/abc367_e
// 入次数除去・周期分解・ダブリングを用いて独立実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/functional_graph.md
// 行先・頂点・周期IDの検査はSKIP_RUNTIME_CHECKSで除外。配列容量の上限保護は維持。
// API一覧
// FunctionalGraph() / FunctionalGraph(span<const int> successor): 空/各頂点の行先で構築。
// build(successor): 再構築。頂点0..n-1、各頂点に行先1つ。ラベル順に制約なし。自己辺可。
//   O(n log(n+1))時間・領域。入力走査中に行先を検査。失敗時は旧状態とviewを維持。
// size()/empty(), successor(v), successors(): 頂点数/空判定/行先/const span。O(1)。
// kth_successor(v,uint64_t k): k回進んだ頂点。全uint64_t範囲可。O(log(n+1))。
// distance(from,to): 初めて到達する辺数。到達不能は-1。O(log(n+1))。
// reachable(from,to): distance>=0。O(log(n+1))。
// info(v): CycleInfo {id,entry,depth,position,length}。O(1)。entryは最初の周期頂点、
//   depthは到達辺数、positionはそのentryの周期内添字、lengthは周期長。
// on_cycle(v), cycle_count(), component_size(v): 周期上か/周期数/同じ弱連結成分の頂点数。O(1)。
// cycle(id): successor方向に並ぶ周期頂点のconst span。O(1)。周期ID/始点は構築時の内部順。
// clear(): 空へ。copyは独立所有、copy代入は失敗時旧状態維持。move元は空で再利用可。swapはO(1)。
// viewは成功したbuild/clear/代入/所有権移動まで有効。buildの入力に自身のviewを渡してよい。

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_FUNCTIONAL_GRAPH_DEFINED
#define SIRKIT_FUNCTIONAL_GRAPH_DEFINED
class FunctionalGraph {
public:
    struct CycleInfo { int id, entry, depth, position, length; };
private:
    std::vector<int> next_, depth_, entry_, cycle_id_, position_, component_size_, cycle_vertices_, jump_;
    std::vector<std::size_t> cycle_offsets_;
    unsigned levels_ = 0;
    void check_vertex(int v) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || static_cast<std::size_t>(v) >= next_.size()) throw std::out_of_range("FunctionalGraph vertex");
#else
        (void)v;
#endif
    }
    void initialize(std::span<const int> successors) {
        const auto n = successors.size();
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max())) throw std::length_error("FunctionalGraph size");
        next_.assign(successors.begin(), successors.end());
        std::vector<int> indegree(n), order; order.reserve(n);
        for (const int v : next_) {
#ifndef SKIP_RUNTIME_CHECKS
            if (v < 0 || static_cast<std::size_t>(v) >= n) throw std::out_of_range("FunctionalGraph successor");
#endif
            ++indegree[v];
        }
        for (std::size_t v = 0; v < n; ++v) if (!indegree[v]) order.push_back(static_cast<int>(v));
        for (std::size_t i = 0; i < order.size(); ++i) if (--indegree[next_[order[i]]] == 0) order.push_back(next_[order[i]]);
        depth_.resize(n); entry_.resize(n); position_.resize(n); cycle_id_.assign(n, -1);
        cycle_offsets_.push_back(0);
        for (std::size_t i = 0; i < n; ++i) if (indegree[i] && cycle_id_[i] == -1) {
            const auto id = static_cast<int>(component_size_.size());
            int v = static_cast<int>(i), position = 0;
            do {
                cycle_id_[v] = id; entry_[v] = v; position_[v] = position++;
                cycle_vertices_.push_back(v); v = next_[v];
            } while (v != static_cast<int>(i));
            component_size_.push_back(position); cycle_offsets_.push_back(cycle_vertices_.size());
        }
        int max_depth = 0;
        for (auto i = order.size(); i != 0; --i) {
            const int v = order[i - 1], p = next_[v];
            depth_[v] = depth_[p] + 1; entry_[v] = entry_[p]; cycle_id_[v] = cycle_id_[p]; position_[v] = position_[p];
            ++component_size_[cycle_id_[v]]; max_depth = std::max(max_depth, depth_[v]);
        }
        levels_ = std::bit_width(static_cast<unsigned>(max_depth));
        if (levels_ && n > jump_.max_size() / levels_) throw std::length_error("FunctionalGraph lifting capacity");
        jump_.resize(n * levels_);
        if (levels_) std::copy(next_.begin(), next_.end(), jump_.begin());
        for (unsigned level = 1; level < levels_; ++level) {
            const auto before = n * (level - 1), here = n * level;
            for (std::size_t v = 0; v < n; ++v) jump_[here + v] = jump_[before + static_cast<std::size_t>(jump_[before + v])];
        }
    }
    int jump_tree(int v, std::uint64_t k) const noexcept {
        const auto n = next_.size();
        for (unsigned level = 0; k; ++level, k >>= 1) if (k & 1) v = jump_[n * level + static_cast<std::size_t>(v)];
        return v;
    }
public:
    FunctionalGraph() = default;
    explicit FunctionalGraph(std::span<const int> successors) { initialize(successors); }
    FunctionalGraph(const FunctionalGraph&) = default;
    FunctionalGraph& operator=(const FunctionalGraph& other) { if (this != &other) { FunctionalGraph copy(other); swap(copy); } return *this; }
    FunctionalGraph(FunctionalGraph&& other) noexcept { swap(other); }
    FunctionalGraph& operator=(FunctionalGraph&& other) noexcept { if (this != &other) { clear(); swap(other); } return *this; }
    void swap(FunctionalGraph& other) noexcept {
        next_.swap(other.next_); depth_.swap(other.depth_); entry_.swap(other.entry_); cycle_id_.swap(other.cycle_id_);
        position_.swap(other.position_); component_size_.swap(other.component_size_); cycle_vertices_.swap(other.cycle_vertices_);
        jump_.swap(other.jump_); cycle_offsets_.swap(other.cycle_offsets_); std::swap(levels_, other.levels_);
    }
    friend void swap(FunctionalGraph& a, FunctionalGraph& b) noexcept { a.swap(b); }
    void build(std::span<const int> successors) { FunctionalGraph next(successors); swap(next); }
    int size() const noexcept { return static_cast<int>(next_.size()); }
    bool empty() const noexcept { return next_.empty(); }
    int successor(int v) const { check_vertex(v); return next_[v]; }
    std::span<const int> successors() const noexcept { return next_; }
    int cycle_count() const noexcept { return static_cast<int>(component_size_.size()); }
    bool on_cycle(int v) const { check_vertex(v); return depth_[v] == 0; }
    int component_size(int v) const { check_vertex(v); return component_size_[cycle_id_[v]]; }
    std::span<const int> cycle(int id) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (id < 0 || id >= cycle_count()) throw std::out_of_range("FunctionalGraph cycle");
#endif
        const auto first = cycle_offsets_[id], last = cycle_offsets_[id + 1];
        return std::span<const int>(cycle_vertices_).subspan(first, last - first);
    }
    CycleInfo info(int v) const {
        check_vertex(v); const int id = cycle_id_[v];
        return {id, entry_[v], depth_[v], position_[v], static_cast<int>(cycle_offsets_[id + 1] - cycle_offsets_[id])};
    }
    int kth_successor(int v, std::uint64_t k) const {
        check_vertex(v);
        const auto depth = static_cast<std::uint64_t>(depth_[v]);
        if (k < depth) return jump_tree(v, k);
        const auto id = cycle_id_[v];
        const auto length = cycle_offsets_[id + 1] - cycle_offsets_[id];
        const auto offset = (static_cast<std::size_t>((k - depth) % length) + static_cast<std::size_t>(position_[v])) % length;
        return cycle_vertices_[cycle_offsets_[id] + offset];
    }
    std::int64_t distance(int from, int to) const {
        check_vertex(from); check_vertex(to);
        if (cycle_id_[from] != cycle_id_[to] || depth_[from] < depth_[to]) return -1;
        if (depth_[to]) {
            const auto d = static_cast<std::uint64_t>(depth_[from] - depth_[to]);
            return jump_tree(from, d) == to ? static_cast<std::int64_t>(d) : -1;
        }
        const auto id = cycle_id_[from];
        const auto length = static_cast<std::int64_t>(cycle_offsets_[id + 1] - cycle_offsets_[id]);
        const auto offset = (static_cast<std::int64_t>(position_[to]) - position_[from] + length) % length;
        return static_cast<std::int64_t>(depth_[from]) + offset;
    }
    bool reachable(int from, int to) const { return distance(from, to) >= 0; }
    void clear() noexcept {
        next_.clear(); depth_.clear(); entry_.clear(); cycle_id_.clear(); position_.clear(); component_size_.clear();
        cycle_vertices_.clear(); jump_.clear(); cycle_offsets_.clear(); levels_ = 0;
    }
};
#endif
