#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/bipartitematching

// Library: Graph/Bipartite_Matching.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Bipartite_Matching.cpp
// 数学的参考: https://doi.org/10.1137/0202019
// 参考（距離ラベルによる再割当）: https://judge.yosupo.jp/submission/339108
// 参考: https://ei1333.github.io/library/graph/flow/bipartite-matching.hpp.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bipartite_matching.md
//
// API一覧
// BipartiteMatching(left=0,right=0): 左0..left-1、右0..right-1を別々に番号付け。O(left+right)。
// add_edge(l,r): 左lと右rの辺を追加。多重辺可、償却O(1)。現在の対応は保ち最大性を無効化。
// reserve_neighbors(l,count): 左lの隣接列を予約。再確保時は次数に比例。neighbors(l): const span。
// solve(): 現在の対応から最大マッチングへ拡張し、総本数を返す。
// 空の対応では両順序の貪欲法で非孤立の左頂点をすべて対応できれば確定し、それ以外は次数1の辺を先に確定する。
// 空の対応で最大次数2以下ならパス・サイクルをO(V+m)で解く。他は次数分布と非孤立頂点数で再割当の左右を選ぶ。
// 対応が空で次数1がなく、次数2の右頂点が多い場合は森を縮約してから解く。
// 総作業量が線形予算に達したら反復Hopcroft-Karp法へ進む。公開APIの左右番号は変えない。
// V=left+right、m=辺数としてO((V+m)*sqrt(V+1))時間・追加O(V+m)領域。最大性が既知ならO(1)。
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


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BIPARTITE_MATCHING_DEFINED
#define SIRKIT_BIPARTITE_MATCHING_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline void CheckBipartiteEdgeCount(std::size_t edges) {
    if (edges == std::numeric_limits<std::size_t>::max())
        throw std::length_error("BipartiteMatching edge count");
}

inline std::pair<std::size_t, std::size_t> BipartiteWorkBudget(std::size_t edges, std::size_t vertices) {
    constexpr std::size_t multiplier = 64;
    constexpr auto maximum = std::numeric_limits<std::size_t>::max();
    const auto unit = edges > maximum - vertices ? maximum : edges + vertices;
    return {unit, unit > maximum / multiplier ? maximum : unit * multiplier};
}
}
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
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || v >= left_size()) throw std::out_of_range("BipartiteMatching left vertex");
#else
        static_cast<void>(v);
#endif
    }
    void check_right(int v) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || v >= right_size()) throw std::out_of_range("BipartiteMatching right vertex");
#else
        static_cast<void>(v);
#endif
    }
    VertexSet vertex_set(bool independent) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (!maximum_) throw std::logic_error("BipartiteMatching solve required");
#endif
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
                // 最大対応上の探索なので未対応の右頂点には届かない。
                // 対応する左頂点に届く道は、今初めて通るこの対応辺に限られる。
                reachable_left[next] = 1; queue.push_back(next);
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
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckBipartiteEdgeCount(edges_);
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
    int solve() { return solve_impl(true); }

private:
    int solve_impl(bool allow_contraction) {
        if (maximum_) return cardinality_;
        if (cardinality_ == std::min(left_size(), right_size())) { maximum_ = true; return cardinality_; }
        if (cardinality_ == 0) {
            // 非孤立の左頂点をすべて対応できれば、追加領域なしで上限に達する。
            const int left_count = left_size();
            auto saturate_left = [&]<bool Reverse>() {
                int matched = 0, i = 0;
                for (; i < left_count; ++i) {
                    const int u = Reverse ? left_count - 1 - i : i;
                    const auto& row = graph_[u];
                    if (row.empty()) continue;
                    for (std::size_t j = 0; j < row.size(); ++j) {
                        const int v = row[Reverse ? row.size() - 1 - j : j];
                        if (right_[v] != -1) continue;
                        left_[u] = v;
                        right_[v] = u;
                        ++matched;
                        break;
                    }
                    if (left_[u] == -1) break;
                }
                if (i == left_count) return matched;
                // 後続の確保や再割当の前に、今回書き込んだ対応をすべて戻す。
                for (int j = 0; j < i; ++j) {
                    const int u = Reverse ? left_count - 1 - j : j;
                    const int v = std::exchange(left_[u], -1);
                    if (v != -1) right_[v] = -1;
                }
                return -1;
            };
            int matched = saturate_left.template operator()<false>();
            if (matched == -1) matched = saturate_left.template operator()<true>();
            if (matched != -1) {
                cardinality_ = matched;
                maximum_ = true;
                return cardinality_;
            }
        }
        const auto n = left_.size();
        // 左右のCSRはsolveだけの作業領域。公開neighborsの格納先は変更しない。
        std::vector<std::size_t> offsets(right_.size() + 1);
        int active_left = 0, active_right = 0;
        // 向き選択だけに使う概算。丸めは結果や計算量保証に影響しない。
        double left_degree_cost = 0, right_degree_cost = 0;
        bool degree_two = true, has_leaf = false;
        std::size_t degree_two_right = 0;
        for (const auto& row : graph_) {
            active_left += !row.empty();
            degree_two = degree_two && row.size() <= 2;
            has_leaf = has_leaf || row.size() == 1;
            const auto degree = static_cast<double>(row.size());
            left_degree_cost += degree * degree;
            for (int v : row) ++offsets[v + 1];
        }
        for (std::size_t v = 0; v < right_.size(); ++v) {
            active_right += offsets[v + 1] != 0;
            degree_two = degree_two && offsets[v + 1] <= 2;
            has_leaf = has_leaf || offsets[v + 1] == 1;
            degree_two_right += offsets[v + 1] == 2;
            const auto degree = static_cast<double>(offsets[v + 1]);
            right_degree_cost += degree * degree;
            offsets[v + 1] += offsets[v];
        }
        const int matching_bound = std::min(active_left, active_right);
        if (cardinality_ == matching_bound) { maximum_ = true; return cardinality_; }
        // 二乗和に2倍超の差があれば押す側の負荷を優先し、近ければ非孤立頂点が少ない側から押す。
        const bool transpose = right_degree_cost > 2.0 * left_degree_cost ||
            (left_degree_cost <= 2.0 * right_degree_cost && active_left < active_right);
        const auto label_size = transpose ? right_.size() : left_.size();
        const auto push_size = transpose ? left_.size() : right_.size();
        std::vector<int> distance(std::max(n, label_size)), queue, path_left, path_right;
        // PR中は左CSRのoffset、HKへ進んだら辺の走査位置として使う。
        std::vector<std::size_t> next(n + 1);
        queue.reserve(distance.size()); path_left.reserve(n); path_right.reserve(std::max(n, right_.size()));
        auto cursor = offsets;
        std::vector<int> reverse_edges(edges_), forward_edges(edges_), active(push_size);
        std::size_t position = 0;
        for (int u = 0; u < left_size(); ++u) {
            next[u] = position;
            for (int v : graph_[u]) {
                reverse_edges[cursor[v]++] = u;
                forward_edges[position++] = v;
            }
        }
        next[n] = position;
        constexpr std::size_t kDegreeTwoShare = 4, kMinimumShrinkShare = 8;
        if (allow_contraction && cardinality_ == 0 && !degree_two && !has_leaf &&
            degree_two_right >= (static_cast<std::size_t>(active_right) + kDegreeTwoShare - 1) / kDegreeTwoShare) {
            // 次数2の右頂点を左2点を結ぶ辺とみなし、その全域森を縮約する。
            std::vector<int> component(n, -1);
            // 縮約を試す間は、距離の領域に各左頂点の親辺を記録する。
            std::fill_n(distance.begin(), n, -1);
            std::vector<unsigned char> tree(right_.size());
            for (int u = 0; u < left_size(); ++u) {
                if (component[u] != -1 || next[u] == next[u + 1]) continue;
                const int id = static_cast<int>(path_left.size());
                path_left.push_back(u);
                component[u] = id;
                queue.clear();
                queue.push_back(u);
                for (std::size_t head = 0; head < queue.size(); ++head) {
                    const int a = queue[head];
                    for (std::size_t i = next[a]; i < next[a + 1]; ++i) {
                        const int v = forward_edges[i];
                        if (offsets[v + 1] - offsets[v] != 2) continue;
                        const auto first = offsets[v];
                        const int other = reverse_edges[first] == a ? reverse_edges[first + 1] : reverse_edges[first];
                        if (component[other] != -1) continue;
                        tree[v] = 1;
                        distance[other] = v;
                        component[other] = id;
                        queue.push_back(other);
                    }
                }
            }
            const auto merged = static_cast<std::size_t>(active_left) - path_left.size();
            // 平行辺などで縮約が小さいと、別グラフを作る費用に見合わない。
            if (merged >= (static_cast<std::size_t>(active_left) + kMinimumShrinkShare - 1) / kMinimumShrinkShare) {
                for (int v = 0; v < right_size(); ++v) {
                    if (tree[v] || offsets[v] == offsets[v + 1]) continue;
                    cursor[v] = path_right.size();
                    path_right.push_back(v);
                }
                BipartiteMatching reduced(static_cast<int>(path_left.size()), static_cast<int>(path_right.size()));
                {
                    std::vector<std::size_t> sizes(path_left.size());
                    for (int v : path_right)
                        for (auto i = offsets[v]; i < offsets[v + 1]; ++i)
                            ++sizes[component[reverse_edges[i]]];
                    for (std::size_t id = 0; id < sizes.size(); ++id)
                        reduced.reserve_neighbors(static_cast<int>(id), sizes[id]);
                }
                for (int u = 0; u < left_size(); ++u)
                    for (std::size_t i = next[u]; i < next[u + 1]; ++i) {
                        const int v = forward_edges[i];
                        if (!tree[v]) reduced.add_edge(component[u], static_cast<int>(cursor[v]));
                    }
                // 各木は内部で頂点数-1本と、外部に最大1本を使える。
                const int outside = reduced.solve_impl(false);
                // 縮約先のsolve終了後は確保を行わない。
                // 外部辺の左端点を根にし、元の根までの木の辺だけ対応を逆向きにする。
                for (std::size_t id = 0; id < path_left.size(); ++id) {
                    const int match = reduced.left_[id];
                    if (match == -1) continue;
                    const int v = path_right[match];
                    // 縮約グラフの辺を作った元の端点が、この右頂点のCSR内にある。
                    std::size_t i = offsets[v];
                    while (component[reverse_edges[i]] != static_cast<int>(id)) ++i;
                    int u = reverse_edges[i];
                    int edge = v;
                    while (true) {
                        left_[u] = edge;
                        right_[edge] = u;
                        const int parent_edge = distance[u];
                        distance[u] = -1;
                        if (parent_edge == -1) break;
                        const auto first = offsets[parent_edge];
                        u = reverse_edges[first] == u ? reverse_edges[first + 1] : reverse_edges[first];
                        edge = parent_edge;
                    }
                }
                // 根への経路を通らなかった頂点は、元の親辺で対応させる。
                for (int u = 0; u < left_size(); ++u) if (distance[u] != -1) {
                    const int v = distance[u];
                    left_[u] = v;
                    right_[v] = u;
                }
                cardinality_ = outside + static_cast<int>(merged);
                maximum_ = true;
                return cardinality_;
            }
            path_left.clear();
        }

        if (cardinality_ == 0 && (has_leaf || degree_two)) {
            // 次数1の辺は最大解に含めてよい。次数2以下で残る成分は偶サイクルなので1辺から切り開く。
            // CSRのcursorとHK用のpath配列を再利用。次数は辺数と同じsize_tで持つ。
            std::vector<std::size_t> degree_left(n);
            for (int u = 0; u < left_size(); ++u) {
                degree_left[u] = graph_[u].size();
                if (degree_left[u] == 1) path_left.push_back(u);
            }
            for (int v = 0; v < right_size(); ++v) {
                cursor[v] = offsets[v + 1] - offsets[v];
                if (cursor[v] == 1) path_right.push_back(v);
            }
            std::size_t core = 0;
            for (;;) {
                int u = -1, v = -1;
                while (!path_left.empty()) {
                    const int a = path_left.back(); path_left.pop_back();
                    if (degree_left[a] != 1) continue;
                    std::size_t i = next[a];
                    while (cursor[forward_edges[i]] == 0) ++i;
                    u = a; v = forward_edges[i];
                    break;
                }
                if (u == -1) while (!path_right.empty()) {
                    const int a = path_right.back(); path_right.pop_back();
                    if (cursor[a] != 1) continue;
                    std::size_t i = offsets[a];
                    while (degree_left[reverse_edges[i]] == 0) ++i;
                    u = reverse_edges[i]; v = a;
                    break;
                }
                if (u == -1) {
                    if (!degree_two) break; // 一般のcoreは既存の再割当・HKへ渡す。
                    while (core < n && degree_left[core] == 0) ++core;
                    if (core == n) break;
                    u = static_cast<int>(core);
                    // 残る次数2頂点には、元の2本がどちらも残っている。
                    v = forward_edges[next[u]];
                }
                degree_left[u] = 0; cursor[v] = 0;
                left_[u] = v; right_[v] = u;
                if (++cardinality_ == matching_bound) { maximum_ = true; return cardinality_; }
                for (std::size_t i = next[u]; i < next[u + 1]; ++i) {
                    const int a = forward_edges[i];
                    if (cursor[a] != 0 && --cursor[a] == 1) path_right.push_back(a);
                }
                for (std::size_t i = offsets[v]; i < offsets[v + 1]; ++i) {
                    const int a = reverse_edges[i];
                    if (degree_left[a] != 0 && --degree_left[a] == 1) path_left.push_back(a);
                }
            }
            bool exhausted = true;
            for (auto degree : degree_left) exhausted = exhausted && degree == 0;
            if (exhausted) { maximum_ = true; return cardinality_; }
        }
        const auto vertices = left_.size() + right_.size();
        auto [unit, remaining] = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::BipartiteWorkBudget(edges_, vertices);
        // 対応配列やgraph_は交換しない。押す側とラベルを付ける側の参照だけを反転する。
        auto reassign = [&]<bool Transpose>() {
            auto& labels = Transpose ? right_ : left_;
            auto& pushes = Transpose ? left_ : right_;
            const int label_count = static_cast<int>(labels.size());
            const int push_count = static_cast<int>(pushes.size());
            auto reverse_neighbors = [&](int v) {
                return std::span<const int>(reverse_edges).subspan(offsets[v], offsets[v + 1] - offsets[v]);
            };
            auto forward_neighbors = [&](int u) {
                return std::span<const int>(forward_edges).subspan(next[u], next[u + 1] - next[u]);
            };
            auto label_neighbors = [&](int u) -> std::span<const int> {
                if constexpr (Transpose) return reverse_neighbors(u);
                else return forward_neighbors(u);
            };
            auto push_neighbors = [&](int v) -> std::span<const int> {
                if constexpr (Transpose) return forward_neighbors(v);
                else return reverse_neighbors(v);
            };
            auto relabel = [&] {
                std::fill_n(distance.begin(), labels.size(), label_count);
                queue.clear();
                for (int u = 0; u < label_count; ++u) if (labels[u] == -1) {
                    distance[u] = 0; queue.push_back(u);
                }
                for (std::size_t head = 0; head < queue.size(); ++head) {
                    int u = queue[head];
                    for (int v : label_neighbors(u)) {
                        int mate = pushes[v];
                        if (mate != -1 && distance[mate] == label_count) {
                            distance[mate] = distance[u] + 1; queue.push_back(mate);
                        }
                    }
                }
            };
            remaining -= unit; relabel();
            std::size_t head = 0, tail = 0, pending = 0;
            for (int v = 0; v < push_count; ++v) if (pushes[v] == -1) active[pending++] = v;
            tail = pending == pushes.size() ? 0 : pending;
            std::size_t processed = 0;
            while (pending != 0) {
                int v = active[head];
                const auto adjacent = push_neighbors(v);
                const auto work = adjacent.size() + 1;
                const bool relabel_due = processed == vertices;
                const auto relabel_work = relabel_due ? unit : 0;
                // 再ラベル後の残予算で次の走査もできる場合だけ、再ラベルする。
                const auto available = remaining - std::min(remaining, relabel_work);
                if (work > available) break;
                remaining = available - work;
                if (relabel_due) { relabel(); processed = 0; }
                if (++head == pushes.size()) head = 0;
                --pending; ++processed;
                int chosen = -1, best = label_count;
                for (int u : adjacent) {
                    if (distance[u] < best) { best = distance[u]; chosen = u; }
                }
                if (chosen == -1) continue;
                int old = labels[chosen];
                if (old != -1) {
                    pushes[old] = -1; active[tail] = old;
                    if (++tail == pushes.size()) tail = 0;
                    ++pending;
                } else ++cardinality_;
                labels[chosen] = v; pushes[v] = chosen; ++distance[chosen];
                if (cardinality_ == matching_bound) return;
            }
        };
        if (transpose) reassign.template operator()<true>();
        else reassign.template operator()<false>();
        if (cardinality_ == matching_bound) { maximum_ = true; return cardinality_; }
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
            for (int start = 0; start < left_size(); ++start) if (left_[start] == -1) {
                path_left.clear(); path_right.clear(); path_left.push_back(start);
                while (!path_left.empty()) {
                    const int u = path_left.back(); bool descended = false, augmented = false;
                    while (next[u] < graph_[u].size()) {
                        const int v = graph_[u][next[u]++], mate = right_[v];
                        if (mate == -1) {
                            // 右の未対応集合は増加で縮む。ここで届く未対応頂点はBFSの最短層にある。
                            left_[u] = v; right_[v] = u; distance[u] = -1;
                            for (std::size_t i = path_right.size(); i > 0; --i) {
                                const int previous = path_left[i - 1], chosen = path_right[i - 1];
                                left_[previous] = chosen; right_[chosen] = previous; distance[previous] = -1;
                            }
                            ++cardinality_;
                            if (cardinality_ == matching_bound) { maximum_ = true; return cardinality_; }
                            augmented = true; break;
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

public:
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

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
// Verify I/O: Verify/current/common/fast_io.hpp
// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_VERIFY_FAST_IO_DEFINED
#define SIRKIT_VERIFY_FAST_IO_DEFINED

// VerifyInput(file=stdin): 非所有のFILEから読む。別の入力方式と混用しない。
// >> 整数: 64bit以下の十進整数を読む。検査省略時は値域内の正しい入力を前提とする。
// >> char/string: 空白を飛ばし、1文字／空白までの文字列を読む。
// >> 浮動小数点: 十進文字列を変換する。通常のCロケールで使う。
// bool / !: 直前までの読み取りが成功しているか／失敗したかを返す。
class VerifyInput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<unsigned char, buffer_size + 1> buffer_;
    std::size_t position_ = 0, size_ = 0;
    bool good_ = true;

    int peek() {
        if (position_ == size_) {
            size_ = std::fread(buffer_.data(), 1, buffer_size, file_);
            buffer_[size_] = 0; // 補充後の末尾は常に数字でない番兵。
            position_ = 0;
            if (size_ == 0) return EOF;
        }
        return buffer_[position_];
    }

    int skip_space() {
        int c;
        while ((c = peek()) != EOF && c <= ' ') ++position_;
        if (c == EOF) good_ = false;
        return c;
    }

public:
    explicit VerifyInput(std::FILE* file = stdin) : file_(file) {}
    VerifyInput(const VerifyInput&) = delete;
    VerifyInput& operator=(const VerifyInput&) = delete;

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyInput& operator>>(T& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        bool negative = false;
        if (c == '-' || c == '+') {
            negative = c == '-';
            ++position_;
            c = peek();
        }
        if (c == EOF) {
            good_ = false;
            return *this;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (c < '0' || c > '9') {
            good_ = false;
            return *this;
        }
        // 先頭の0を除けば、20桁以下の末尾直前の値はuint64_tに収まる。
        while (c == '0') {
            ++position_;
            c = peek();
        }
        std::uint64_t previous = 0;
        unsigned last_digit = 0;
        std::size_t digits = 0;
#endif
        std::uint64_t value = 0;
#ifdef SKIP_RUNTIME_CHECKS
        // バッファ末尾の番兵で内側の範囲判定を省き、数字が境界をまたぐときだけ補充する。
        for (;;) {
            const unsigned char* first = buffer_.data() + position_;
            const unsigned char* cursor = first;
            while (static_cast<unsigned>(*cursor - '0') < 10) {
                value = value * 10 + static_cast<unsigned>(*cursor - '0');
                ++cursor;
            }
            position_ += static_cast<std::size_t>(cursor - first);
            if (position_ != size_) break;
            c = peek();
            if (c == EOF) break;
        }
#else
        while (c >= '0' && c <= '9') {
            previous = value;
            last_digit = static_cast<unsigned>(c - '0');
            if (digits <= 20) ++digits;
            value = value * 10 + static_cast<unsigned>(c - '0');
            ++position_;
            c = peek();
        }
#endif
#ifndef SKIP_RUNTIME_CHECKS
        constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
        if ((c != EOF && c > ' ') || digits > 20
            || (digits == 20 && (previous > maximum / 10
                || (previous == maximum / 10 && last_digit > maximum % 10)))) {
            good_ = false;
            return *this;
        }
        std::uint64_t limit = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
        if constexpr (std::is_signed_v<T>) limit += negative;
        else if (negative && value != 0) good_ = false;
        if (value > limit) good_ = false;
        if (!good_) return *this;
#endif
        if constexpr (std::is_signed_v<T>) {
            constexpr std::uint64_t minimum_magnitude =
                static_cast<std::uint64_t>(std::numeric_limits<T>::max()) + 1;
            // -(最小値)は評価せず、絶対値が最大値+1のときだけ最小値を使う。
            result = negative
                ? (value == minimum_magnitude ? std::numeric_limits<T>::min()
                                  : static_cast<T>(-static_cast<T>(value)))
                : static_cast<T>(value);
        } else result = static_cast<T>(value);
        return *this;
    }

    VerifyInput& operator>>(char& result) {
        if (!good_) return *this;
        const int c = skip_space();
        if (c != EOF) {
            result = static_cast<char>(c);
            ++position_;
        }
        return *this;
    }

    VerifyInput& operator>>(std::string& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        result.clear();
        for (;;) {
            const std::size_t first = position_;
            while (position_ < size_ && buffer_[position_] > ' ') ++position_;
            result.append(reinterpret_cast<const char*>(buffer_.data() + first), position_ - first);
            if (position_ != size_ || peek() == EOF) break;
        }
        return *this;
    }

    template<class T> requires std::is_floating_point_v<T>
    VerifyInput& operator>>(T& result) {
        std::string token;
        *this >> token;
        if (!good_) return *this;
#ifndef SKIP_RUNTIME_CHECKS
        char* end = nullptr;
        const long double value = std::strtold(token.c_str(), &end);
        if (end != token.c_str() + token.size()) good_ = false;
        else result = static_cast<T>(value);
#else
        result = static_cast<T>(std::strtold(token.c_str(), nullptr));
#endif
        return *this;
    }
};

// VerifyOutput(file=stdout): 非所有のFILEへ書く。破棄時にflushし、FILEは閉じない。
// << 整数/char/文字列/浮動小数点: 値を出力する。整数は64bit以下に対応する。
// fixed(digits): 浮動小数点を小数点以下digits桁で出力する（digits>=0）。
// flush(): バッファの内容をFILEへ渡す。bool / !で書き込み失敗を確認できる。
class VerifyOutput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<char, buffer_size> buffer_;
    std::size_t size_ = 0;
    int precision_ = 6;
    bool fixed_ = false, good_ = true;

    void write(std::string_view text) {
        while (!text.empty() && good_) {
            if (size_ == buffer_.size()) flush();
            const auto count = std::min(text.size(), buffer_.size() - size_);
            std::memcpy(buffer_.data() + size_, text.data(), count);
            size_ += count;
            text.remove_prefix(count);
        }
    }

public:
    explicit VerifyOutput(std::FILE* file = stdout) : file_(file) {}
    VerifyOutput(const VerifyOutput&) = delete;
    VerifyOutput& operator=(const VerifyOutput&) = delete;
    ~VerifyOutput() { flush(); }

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    VerifyOutput& fixed(int digits) {
        if (digits < 0) throw std::invalid_argument("negative output precision");
        fixed_ = true;
        precision_ = digits;
        return *this;
    }

    void flush() noexcept {
        std::size_t written = 0;
        while (written < size_ && good_) {
            const auto count = std::fwrite(buffer_.data() + written, 1, size_ - written, file_);
            if (count == 0) good_ = false;
            written += count;
        }
        size_ = 0;
    }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyOutput& operator<<(T value) {
        if constexpr (std::is_same_v<T, bool>) return *this << static_cast<char>('0' + value);
        else {
            constexpr std::size_t capacity = std::numeric_limits<std::uint64_t>::digits10 + 3;
            if (buffer_.size() - size_ < capacity) flush();
            if (!good_) return *this;
            const auto end = std::to_chars(buffer_.data() + size_, buffer_.data() + size_ + capacity, value).ptr;
            size_ = static_cast<std::size_t>(end - buffer_.data());
            return *this;
        }
    }

    VerifyOutput& operator<<(char value) {
        if (size_ == buffer_.size()) flush();
        if (good_) buffer_[size_++] = value;
        return *this;
    }

    VerifyOutput& operator<<(std::string_view value) { write(value); return *this; }
    VerifyOutput& operator<<(const std::string& value) { write(value); return *this; }
    VerifyOutput& operator<<(const char* value) { write(value); return *this; }

    template<class T> requires std::is_floating_point_v<T>
    VerifyOutput& operator<<(T value) {
        char local[128];
        const char* format = fixed_ ? "%.*Lf" : "%.*Lg";
        const int count = std::snprintf(local, sizeof(local), format, precision_,
                                        static_cast<long double>(value));
        if (count < 0) good_ = false;
        else if (static_cast<std::size_t>(count) < sizeof(local))
            write(std::string_view(local, static_cast<std::size_t>(count)));
        else {
            std::vector<char> text(static_cast<std::size_t>(count) + 1);
            std::snprintf(text.data(), text.size(), format, precision_, static_cast<long double>(value));
            write(std::string_view(text.data(), static_cast<std::size_t>(count)));
        }
        return *this;
    }
};

inline VerifyInput verify_in;
inline VerifyOutput verify_out;
#endif

int main() {
    int left{},right{},edges{}; verify_in >> left >> right >> edges;
    BipartiteMatching graph(left,right);
    for (int i=0;i<edges;++i) { int u{},v{}; verify_in >> u >> v; graph.add_edge(u,v); }
    verify_out << graph.solve() << '\n';
    for (auto [u,v] : graph.pairs()) verify_out << u << ' ' << v << '\n';
}
