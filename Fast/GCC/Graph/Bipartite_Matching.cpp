// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Bipartite_Matching.cpp
// 数学的参考: https://doi.org/10.1137/0202019
// 参考: https://ei1333.github.io/library/graph/flow/bipartite-matching.hpp.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bipartite_matching.md
//
// API一覧
// BipartiteMatching(left=0,right=0): 左0..left-1、右0..right-1を別々に番号付け。O(left+right)。
// add_edge(l,r): 左lと右rの辺を追加。多重辺可、償却O(1)。現在の対応は保ち最大性を無効化。
// reserve_neighbors(l,count): 左lの隣接列を予約。再確保時は次数に比例。neighbors(l): const span。
// solve(): 現在の対応から最大マッチングへ拡張し、総本数を返す。反復Hopcroft-Karp法。
// V=left+right、m=辺数としてO((V+m)*sqrt(V+1))時間・追加O(left)領域。最大性が既知ならO(1)。
// left_size()/right_size()/edge_count()/matching_size()/is_maximum()/empty(): O(1)。emptyは両頂点集合が空。
// match_left(l)/match_right(r): 対応先、未対応なら-1。left_matches()/right_matches(): const span。
// pairs(): 左番号順の対応辺をvectorで返す。O(left)時間・O(matching_size())領域。
// VertexSet{vector<int> left,right; size()}: 左右の頂点集合と合計件数。
// min_vertex_cover()/max_independent_set(): 最小頂点被覆/最大独立集合、O(V+m)時間・領域。
// 被覆・独立集合はis_maximum()==trueが前提。多重辺は本数に数えるが、対応・被覆の意味は同じ。
// clear_matching(): 辺を保ち対応を消す、O(V)。clear_edges(): 頂点数・容量を保ち辺と対応を消す、O(V+m)。
// reset(left=0,right=0)/clear(): 空の指定頂点数/0頂点へ戻す。swapはO(1)。コピーは独立、move元は0頂点。
// solveの作業領域確保は更新前に行い、確保失敗時は対応を維持。add_edge/reset/コピー代入にも強い例外保証。
// spanはreset/clear/代入で無効化。隣接spanは対応するadd_edge/reserveでも無効化し得る。
// SKIP_RUNTIME_CHECKSで端点・最大性の入力検査を除く。件数/内部添字の表現上限は維持する。

#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_BIPARTITE_MATCHING_DEFINED
#define SIRKIT_BIPARTITE_MATCHING_DEFINED
class BipartiteMatching {
public:
    struct VertexSet {
        std::vector<int> left, right;
        std::size_t size() const noexcept { return left.size() + right.size(); }
    };
private:
    std::vector<std::vector<int>> graph_;
    std::vector<int> left_, right_;
    std::size_t edges_ = 0;
    int cardinality_ = 0;
    bool maximum_ = true;

    static std::size_t count(int n) {
        if (n < 0) throw std::length_error("BipartiteMatching vertex count");
        return static_cast<std::size_t>(n);
    }
    void check_left(int v) const {
        static_cast<void>(v);
    }
    void check_right(int v) const {
        static_cast<void>(v);
    }
    VertexSet vertex_set(bool independent) const {
        std::vector<unsigned char> reachable_left(left_.size()), reachable_right(right_.size());
        std::vector<int> queue; queue.reserve(left_.size());
        for (int u = 0; u < left_size(); ++u) if (left_[u] == -1) {
            reachable_left[u] = 1; queue.push_back(u);
        }
        for (std::size_t head = 0; head < queue.size(); ++head) {
            const int u = queue[head];
            for (const int v : graph_[u]) if (v != left_[u] && !reachable_right[v]) {
                reachable_right[v] = 1;
                const int next = right_[v];
                if (next != -1 && !reachable_left[next]) {
                    reachable_left[next] = 1; queue.push_back(next);
                }
            }
        }
        VertexSet result;
        for (int u = 0; u < left_size(); ++u)
            if (static_cast<bool>(reachable_left[u]) == independent) result.left.push_back(u);
        for (int v = 0; v < right_size(); ++v)
            if (static_cast<bool>(reachable_right[v]) != independent) result.right.push_back(v);
        return result;
    }
public:
    explicit BipartiteMatching(int left = 0, int right = 0)
        : graph_(count(left)), left_(count(left), -1), right_(count(right), -1) {}
    BipartiteMatching(const BipartiteMatching&) = default;
    BipartiteMatching& operator=(const BipartiteMatching& other) {
        if (this != &other) { BipartiteMatching next(other); swap(next); }
        return *this;
    }
    BipartiteMatching(BipartiteMatching&& other) noexcept { swap(other); }
    BipartiteMatching& operator=(BipartiteMatching&& other) noexcept {
        BipartiteMatching next(std::move(other)); swap(next); return *this;
    }
    void swap(BipartiteMatching& other) noexcept {
        graph_.swap(other.graph_); left_.swap(other.left_); right_.swap(other.right_);
        std::swap(edges_, other.edges_); std::swap(cardinality_, other.cardinality_); std::swap(maximum_, other.maximum_);
    }
    friend void swap(BipartiteMatching& a, BipartiteMatching& b) noexcept { a.swap(b); }
    int left_size() const noexcept { return static_cast<int>(left_.size()); }
    int right_size() const noexcept { return static_cast<int>(right_.size()); }
    bool empty() const noexcept { return left_.empty() && right_.empty(); }
    std::size_t edge_count() const noexcept { return edges_; }
    int matching_size() const noexcept { return cardinality_; }
    bool is_maximum() const noexcept { return maximum_; }
    void add_edge(int left, int right) {
        check_left(left); check_right(right);
        if (edges_ == std::numeric_limits<std::size_t>::max())
            throw std::length_error("BipartiteMatching edge count");
        graph_[left].push_back(right); ++edges_; maximum_ = false;
    }
    void reserve_neighbors(int left, std::size_t capacity) { check_left(left); graph_[left].reserve(capacity); }
    std::span<const int> neighbors(int left) const { check_left(left); return graph_[left]; }
    std::span<const int> left_matches() const noexcept { return left_; }
    std::span<const int> right_matches() const noexcept { return right_; }
    int match_left(int left) const { check_left(left); return left_[left]; }
    int match_right(int right) const { check_right(right); return right_[right]; }
    std::vector<std::pair<int,int>> pairs() const {
        std::vector<std::pair<int,int>> result; result.reserve(static_cast<std::size_t>(cardinality_));
        for (int u = 0; u < left_size(); ++u) if (left_[u] != -1) result.emplace_back(u, left_[u]);
        return result;
    }
    int solve() {
        if (maximum_) return cardinality_;
        if (cardinality_ == std::min(left_size(), right_size())) { maximum_ = true; return cardinality_; }
        const auto n = left_.size();
        std::vector<int> distance(n), queue, path_left, path_right;
        std::vector<std::size_t> next(n);
        queue.reserve(n); path_left.reserve(n); path_right.reserve(n);
        for (;;) {
            std::fill(distance.begin(), distance.end(), -1);
            queue.clear();
            for (int u = 0; u < left_size(); ++u) if (left_[u] == -1) { distance[u] = 0; queue.push_back(u); }
            int shortest = -1;
            for (std::size_t head = 0; head < queue.size(); ++head) {
                const int u = queue[head];
                if (shortest != -1 && distance[u] >= shortest) continue;
                for (const int v : graph_[u]) {
                    const int mate = right_[v];
                    if (mate == -1) { if (shortest == -1) shortest = distance[u] + 1; }
                    else if (distance[mate] == -1) { distance[mate] = distance[u] + 1; queue.push_back(mate); }
                }
            }
            if (shortest == -1) break;
            std::fill(next.begin(), next.end(), 0);
            for (int start = 0; start < left_size(); ++start) if (left_[start] == -1 && distance[start] == 0) {
                path_left.clear(); path_right.clear(); path_left.push_back(start);
                while (!path_left.empty()) {
                    const int u = path_left.back(); bool descended = false, augmented = false;
                    while (next[u] < graph_[u].size()) {
                        const int v = graph_[u][next[u]++], mate = right_[v];
                        if (mate == -1) {
                            if (distance[u] + 1 != shortest) continue;
                            left_[u] = v; right_[v] = u; distance[u] = -1;
                            for (std::size_t i = path_right.size(); i > 0; --i) {
                                const int previous = path_left[i - 1], chosen = path_right[i - 1];
                                left_[previous] = chosen; right_[chosen] = previous; distance[previous] = -1;
                            }
                            ++cardinality_; augmented = true; break;
                        }
                        if (distance[mate] == distance[u] + 1 && distance[mate] < shortest) {
                            path_left.push_back(mate); path_right.push_back(v); descended = true; break;
                        }
                    }
                    if (augmented) break;
                    if (descended) continue;
                    distance[u] = -1; path_left.pop_back();
                    if (!path_right.empty()) path_right.pop_back();
                }
            }
        }
        maximum_ = true; return cardinality_;
    }
    VertexSet min_vertex_cover() const { return vertex_set(false); }
    VertexSet max_independent_set() const { return vertex_set(true); }
    void clear_matching() noexcept {
        std::fill(left_.begin(), left_.end(), -1); std::fill(right_.begin(), right_.end(), -1);
        cardinality_ = 0; maximum_ = edges_ == 0;
    }
    void clear_edges() noexcept { for (auto& adjacent : graph_) adjacent.clear(); edges_ = 0; clear_matching(); }
    void reset(int left = 0, int right = 0) { BipartiteMatching next(left, right); swap(next); }
    void clear() noexcept { graph_.clear(); left_.clear(); right_.clear(); edges_ = 0; cardinality_ = 0; maximum_ = true; }
};
#endif
