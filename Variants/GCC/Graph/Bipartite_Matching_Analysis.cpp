// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Bipartite_Matching_Analysis.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bipartite_matching_analysis.md
//
// API一覧
// analyze_bipartite_matching(left,right,edges): 最大マッチングにおける全辺と全頂点の役割を調べる。
//   edgesは(left_id,right_id)で、左右別々に0始まり。多重辺のIDも入力順で区別する。
//   matching_sizeは最大サイズ。edge_roleは入力辺順のforbidden/optional/mandatory。
//   左右のcan_be_freeは、該当頂点を未対応にする最大マッチングがあるか。
//   n=left+right。最大マッチング1回とO(n+m)の追加処理、領域O(n+m)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNWEIGHTED_GRAPH_DEFINED
#define SIRKIT_UNWEIGHTED_GRAPH_DEFINED
using Graph = std::vector<std::vector<int>>;
#endif

#ifndef SIRKIT_SCC_DETAIL_DEFINED
#define SIRKIT_SCC_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
// 既存CSR用の次数配列を調べる。通常のm!=nでは呼ばれない。
template<class Index>
[[gnu::noinline]]
inline bool outdegrees_all_one(const std::vector<Index>& degree) {
    for(auto it=degree.begin()+1;it!=degree.end();++it)if(*it!=1)return false;
    return true;
}

// 出次数が全て1なら、閉路と流入木を反復的に分解する。
[[gnu::noinline]]
inline std::pair<int,std::vector<int>> functional_component_ids(
        int n,const std::vector<std::pair<int,int>>& edges) {
    constexpr int on_path=std::numeric_limits<int>::min();
    // ~vは[-n,-1]なので、n<=INT_MAXではon_pathと衝突しない。
    std::vector<int> ids(n);
    for(auto [u,v]:edges)ids[u]=~v;
    auto path=std::make_unique_for_overwrite<int[]>(n);
    int top=0,count=0;
    for(int root=0;root<n;++root) {
        if(ids[root]>=0)continue;
        int v=root,state;
        while((state=ids[v])<0&&state!=on_path) {
            ids[v]=on_path;path[top++]=v;v=~state;
        }
        if(state==on_path) {
            int member;
            do{member=path[--top];ids[member]=count;}while(member!=v);
            ++count;
        }
        while(top!=0)ids[path[--top]]=count++;
    }
    for(auto& id:ids)id=count-1-id;
    return {count,std::move(ids)};
}

// Indexは総辺数を表現できる符号なし型。
template<class Index>
inline std::pair<int, std::vector<int>> directed_component_ids_index(
        int n, const std::vector<std::pair<int, int>>& edges) {
    std::vector<Index> begin(static_cast<std::size_t>(n) + 1);
    for (auto [u, v] : edges) { (void)v; ++begin[static_cast<std::size_t>(u) + 1]; }
    if(edges.size()==static_cast<std::size_t>(n) && outdegrees_all_one(begin))
        return functional_component_ids(n,edges);
    for (int v = 0; v < n; ++v) begin[v + 1] += begin[v];
    std::vector<Index> next(begin.begin() + 1, begin.end());
    // CSRの全要素へ書き込んでから参照するため、先行初期化は不要。
    auto destination = std::make_unique_for_overwrite<int[]>(edges.size());
    // 辺順を保って逆向きに埋め、走査位置の再初期化を省く。
    for (auto it = edges.rbegin(); it != edges.rend(); ++it)
        destination[--next[it->first]] = it->second;
    // idsは中断中のlow-linkの退避先を兼ね、確定時に成分番号を書く。
    std::vector<int> entered(n, -1), ids(n), dfs, pending;
    dfs.reserve(n); pending.reserve(n);
    int clock = 0, count = 0;
    for (int root = 0; root < n; ++root) {
        if (entered[root] != -1) continue;
        int v = root;
        Index cursor = next[v], end = begin[v + 1];
        // 出辺0の頂点は即確定し、DFS番号の割当ても省く。
        if (cursor == end) {
            entered[v] = n; ids[v] = count++;
            continue;
        }
        int low = entered[v] = clock++;
        pending.push_back(v);
        for (;;) {
            if (cursor != end) {
                const int u = destination[cursor++];
                if (entered[u] == -1) {
                    const Index child_cursor = next[u], child_end = begin[u + 1];
                    if (child_cursor == child_end) {
                        entered[u] = n; ids[u] = count++;
                        continue;
                    }
                    // 子へ降りるときだけ、親の走査位置とlow-linkを退避する。
                    next[v] = cursor;
                    ids[v] = low;
                    dfs.push_back(v);
                    v = u;
                    low = entered[v] = clock++;
                    cursor = child_cursor; end = child_end;
                    pending.push_back(v);
                } else low = std::min(low, entered[u]);
                continue;
            }
            // 成分番号への上書き後も親へ渡せるよう、low-linkを退避する。
            const int child_low = low;
            if (child_low == entered[v]) {
                for (;;) {
                    const int u = pending.back(); pending.pop_back();
                    entered[u] = n; ids[u] = count;
                    if (u == v) break;
                }
                ++count;
            }
            if (dfs.empty()) break;
            v = dfs.back(); dfs.pop_back();
            low = std::min(ids[v], child_low);
            cursor = next[v]; end = begin[v + 1];
        }
    }
    for (auto& id : ids) id = count - 1 - id;
    return {count, std::move(ids)};
}
template<std::uint32_t kCompactEdgeLimit = std::numeric_limits<std::uint32_t>::max()>
inline std::pair<int, std::vector<int>> directed_component_ids(
        int n, const std::vector<std::pair<int, int>>& edges) {
    // 無辺では各頂点が1成分。従来と同じ逆番号で返す。
    if (edges.empty()) {
        std::vector<int> ids(n);
        for (int v = 0; v < n; ++v) ids[v] = n - 1 - v;
        return {n, std::move(ids)};
    }
    // 辺数が収まる場合だけCSRの添字を32bitにする。
    if (edges.size() <= kCompactEdgeLimit)
        return directed_component_ids_index<std::uint32_t>(n, edges);
    return directed_component_ids_index<std::size_t>(n, edges);
}

inline int SccVertexCount(std::size_t vertices) {
    if (vertices > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("SCC: too many vertices");
    return static_cast<int>(vertices);
}
}
#endif

#ifndef SIRKIT_SCC_GRAPH_DEFINED
#define SIRKIT_SCC_GRAPH_DEFINED
class scc_graph {
    int n_ = 0, count_ = 0;
    std::vector<std::pair<int, int>> edges_;
    std::vector<int> ids_;
#ifndef SKIP_RUNTIME_CHECKS
    bool built_ = false;
    void vertex(int v) const {
        if (v < 0 || v >= n_) throw std::out_of_range("SCC: vertex out of range");
    }
    void require_build() const {
        if (!built_) throw std::logic_error("SCC: call scc or build first");
    }
#endif
public:
    scc_graph() = default;
    explicit scc_graph(int n) : n_(n) {
        if (n < 0) throw std::length_error("SCC: negative size");
    }
    explicit scc_graph(const Graph& g)
        : n_(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SccVertexCount(g.size())) {
        for (int u = 0; u < n_; ++u) for (int v : g[u]) add_edge(u, v);
        build();
    }
    scc_graph(const scc_graph&) = default;
    scc_graph(scc_graph&& other) noexcept { swap(other); }
    scc_graph& operator=(scc_graph other) noexcept { swap(other); return *this; }
    void swap(scc_graph& other) noexcept {
        std::swap(n_, other.n_); std::swap(count_, other.count_);
        edges_.swap(other.edges_); ids_.swap(other.ids_);
#ifndef SKIP_RUNTIME_CHECKS
        std::swap(built_, other.built_);
#endif
    }
    void reset(int n = 0) { scc_graph replacement(n); swap(replacement); }
    int size() const noexcept { return n_; }
    std::size_t edge_count() const noexcept { return edges_.size(); }
    void reserve_edges(std::size_t count) { edges_.reserve(count); }
    void add_edge(int u, int v) {
#ifndef SKIP_RUNTIME_CHECKS
        vertex(u); vertex(v);
#endif
        edges_.emplace_back(u, v);
#ifndef SKIP_RUNTIME_CHECKS
        built_ = false;
#endif
    }
    int build() {
        auto result = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::directed_component_ids(n_, edges_);
        ids_.swap(result.second); count_ = result.first;
#ifndef SKIP_RUNTIME_CHECKS
        built_ = true;
#endif
        return count_;
    }
    int component_count() const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build();
#endif
        return count_;
    }
    int id(int v) const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build(); vertex(v);
#endif
        return ids_[v];
    }
    const std::vector<int>& ids() const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build();
#endif
        return ids_;
    }
    std::vector<std::vector<int>> scc() {
        build();
        std::vector<std::vector<int>> groups(count_);
        // 1成分・全頂点が別成分ならサイズの集計は不要。
        if (count_ == 1) {
            auto& group = groups.front();
            group.resize(n_);
            for (int v = 0; v < n_; ++v) group[v] = v;
            return groups;
        }
        if (count_ == n_) {
            // 確保も成分順に行い、返す頂点群の配置を揃える。
            for (auto& group : groups) group.reserve(1);
            for (int v = 0; v < n_; ++v) groups[ids_[v]].push_back(v);
            return groups;
        }
        std::vector<int> sizes(count_);
        for (int id : ids_) ++sizes[id];
        for (int id = 0; id < count_; ++id) groups[id].reserve(sizes[id]);
        for (int v = 0; v < n_; ++v) groups[ids_[v]].push_back(v);
        return groups;
    }
    Graph condensation(bool unique_edges = true) const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build();
#endif
        Graph result(count_);
        if (!unique_edges) {
            for (auto [u, v] : edges_) if (ids_[u] != ids_[v]) result[ids_[u]].push_back(ids_[v]);
            return result;
        }
        for (auto [u, v] : edges_) if (ids_[u] != ids_[v]) result[ids_[u]].push_back(ids_[v]);
        std::vector<int> last(count_, -1);
        // 同じ行で重複を詰め、頂点群の二重確保を避ける。
        for (int from = 0; from < count_; ++from) {
            auto& row = result[from];
            std::size_t used = 0;
            for (int to : row) if (last[to] != from) {
                last[to] = from; row[used++] = to;
            }
            row.resize(used);
            // 多重辺を大量に除いた行の余分な容量を解放する。
            if (used < row.capacity() / 2) {
                std::vector<int> compact(row);
                row.swap(compact);
            }
        }
        return result;
    }
};
#endif

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

#ifndef SIRKIT_BIPARTITE_MATCHING_ANALYSIS_DEFINED
#define SIRKIT_BIPARTITE_MATCHING_ANALYSIS_DEFINED

enum class BipartiteEdgeRole : std::uint8_t
{
    forbidden,
    optional,
    mandatory,
};

struct BipartiteMatchingAnalysis
{
    int matching_size = 0;
    std::vector<BipartiteEdgeRole> edge_role;
    std::vector<bool> left_can_be_free, right_can_be_free;
};

namespace detail
{
inline std::vector<bool> matching_reachability(
    const Graph &arcs, std::span<const int> matching, int offset)
{
    std::vector<bool> reached(arcs.size());
    std::vector<int> pending;
    pending.reserve(arcs.size());
    for (std::size_t i = 0; i < matching.size(); ++i)
    {
        if (matching[i] == -1)
        {
            const int vertex = offset + static_cast<int>(i);
            reached[vertex] = true;
            pending.push_back(vertex);
        }
    }
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        for (const int vertex : arcs[pending[i]])
        {
            if (!reached[vertex])
            {
                reached[vertex] = true;
                pending.push_back(vertex);
            }
        }
    }
    return reached;
}
} // namespace detail

inline BipartiteMatchingAnalysis analyze_bipartite_matching(
    int left, int right, std::span<const std::pair<int, int>> edges)
{
    if (left < 0 || right < 0)
    {
        throw std::invalid_argument("bipartite matching analysis: negative side size");
    }
    if (left > std::numeric_limits<int>::max() - right)
    {
        throw std::length_error("bipartite matching analysis: too many vertices");
    }
    const int n = left + right;
    BipartiteMatching matching(left, right);
    for (const auto &[u, v] : edges)
    {
        matching.add_edge(u, v);
    }
    BipartiteMatchingAnalysis result;
    result.matching_size = matching.solve();
    const auto left_matches = matching.left_matches();
    const auto right_matches = matching.right_matches();

    Graph arcs(n), reverse(n);
    scc_graph strongly_connected(n);
    strongly_connected.reserve_edges(edges.size());
    std::vector<bool> chosen(edges.size()), has_chosen(left);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, right_index] = edges[id];
        const int v = left + right_index;
        chosen[id] = left_matches[u] == right_index && !has_chosen[u];
        if (chosen[id])
        {
            has_chosen[u] = true;
        }
        const auto [from, to] = chosen[id] ? std::pair{v, u} : std::pair{u, v};
        arcs[from].push_back(to);
        reverse[to].push_back(from);
        strongly_connected.add_edge(from, to);
    }
    strongly_connected.build();
    const auto &component = strongly_connected.ids();
    const auto from_free_left = detail::matching_reachability(arcs, left_matches, 0);
    const auto to_free_right = detail::matching_reachability(reverse, right_matches, left);
    result.left_can_be_free.assign(from_free_left.begin(), from_free_left.begin() + left);
    result.right_can_be_free.assign(to_free_right.begin() + left, to_free_right.end());

    result.edge_role.reserve(edges.size());
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, right_index] = edges[id];
        const int v = left + right_index;
        const auto [from, to] = chosen[id] ? std::pair{v, u} : std::pair{u, v};
        // 最大マッチング間の差は、交互閉路か未対応頂点を移す交互路。
        const bool exchangeable = component[u] == component[v] || from_free_left[from] || to_free_right[to];
        result.edge_role.push_back(exchangeable ? BipartiteEdgeRole::optional
                                   : chosen[id] ? BipartiteEdgeRole::mandatory
                                                : BipartiteEdgeRole::forbidden);
    }
    return result;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
