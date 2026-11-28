// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Graph/Reachability_Chain_Cover.cpp
// 参考（最小チェーン被覆とフローの対応）: https://atcoder.jp/contests/ttpc2022/editorial/5121
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/reachability_chain_cover.md
//
// API一覧
// minimum_reachability_chain_cover(n,edges): 有向グラフを最少個の到達可能性チェーンに分割する。
//   0始まりの辺(from,to)を受け、チェーンごとの頂点列を返す。先の頂点から後の頂点へ到達できる。
//   到達経路は別チェーンの頂点も通れる。自己ループ・多重辺・有向閉路可。
//   強連結成分数c、縮約辺数aとしてO(n+m)の分解とO(c+a)規模の最大流1回。
//   復元はO(n+m+L)、Lは分解した縮約グラフ上の歩道の長さの合計で高々c²。領域O(n+m)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>
#include <span>
#include <stdexcept>
#include <type_traits>
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
inline bool outdegrees_all_one(const std::vector<Index>& degree) {
    for(auto it=degree.begin()+1;it!=degree.end();++it)if(*it!=1)return false;
    return true;
}

// 出次数が全て1なら、閉路と流入木を反復的に分解する。
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

#ifndef SIRKIT_MAX_FLOW_CAPACITY_DEFINED
#define SIRKIT_MAX_FLOW_CAPACITY_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline void CheckMaxFlowAppendCapacity(std::size_t edges, std::size_t from_degree,
                                       std::size_t to_degree, bool loop) {
    constexpr auto maximum = static_cast<std::size_t>(std::numeric_limits<int>::max());
    if (edges >= maximum) throw std::length_error("mf_graph: too many edges");
    if (from_degree >= maximum || to_degree >= maximum || (loop && from_degree >= maximum - 1))
        throw std::length_error("mf_graph: too many incident edges");
}

constexpr bool MaxFlowCanUsePreflow(std::size_t vertices) noexcept {
    return vertices < static_cast<std::size_t>(std::numeric_limits<int>::max());
}
}
#endif
#ifndef SIRKIT_MAX_FLOW_DEFINED
#define SIRKIT_MAX_FLOW_DEFINED

template<class Cap = std::int64_t>
class mf_graph {
    static_assert(std::is_integral_v<Cap> && !std::is_same_v<Cap, bool>, "capacity must be a non-bool integer");
public:
    struct edge { int from; int to; Cap cap; Cap flow; };
private:
    struct Arc { int to; int reverse; Cap residual; };
    struct Frame { int vertex; Cap limit; Cap sent; };
    std::vector<std::vector<Arc>> graph_;
    std::vector<std::pair<int, int>> positions_;
    std::vector<int> level_, next_, queue_;
    std::vector<Frame> frames_;
    static std::size_t checked_size(int n) {
        if (n < 0) throw std::length_error("mf_graph: negative vertex count");
        return static_cast<std::size_t>(n);
    }
#ifndef SKIP_RUNTIME_CHECKS
    void vertex(int v) const {
        if (v < 0 || v >= size()) throw std::out_of_range("mf_graph: vertex out of range");
    }
    void edge_id(int id) const {
        if (id < 0 || static_cast<std::size_t>(id) >= positions_.size()) throw std::out_of_range("mf_graph: edge out of range");
    }
    static void nonnegative(Cap value) {
        if constexpr (std::is_signed_v<Cap>) if (value < 0) throw std::invalid_argument("mf_graph: negative capacity");
    }
#endif
    bool levels(int source, int sink) {
        std::fill(level_.begin(), level_.end(), -1);
        int front = 0, end = 0; queue_[end++] = source; level_[source] = 0;
        while (front != end) {
            const int v = queue_[front++];
            for (const auto& edge : graph_[v]) if (edge.residual != 0 && level_[edge.to] == -1) {
                level_[edge.to] = level_[v] + 1;
                // sinkの直前層までは発見済みなので、最短路の辺はDFSで列挙できる。
                if (edge.to == sink) return true;
                queue_[end++] = edge.to;
            }
        }
        return false;
    }
    Cap blocking_flow(int source, int sink, Cap limit) {
        frames_.clear(); frames_.push_back({source, limit, 0});
        for (;;) {
            auto& frame = frames_.back(); const int v = frame.vertex;
            if (v != sink && frame.sent != frame.limit) {
                auto& cursor = next_[v];
                while (cursor != static_cast<int>(graph_[v].size())) {
                    const auto& edge = graph_[v][cursor];
                    if (edge.residual != 0 && level_[edge.to] == level_[v] + 1) break;
                    ++cursor;
                }
                if (cursor != static_cast<int>(graph_[v].size())) {
                    const auto& edge = graph_[v][cursor];
                    frames_.push_back({edge.to, std::min(static_cast<Cap>(frame.limit - frame.sent), edge.residual), 0});
                    continue;
                }
                level_[v] = -1;
            }
            const Cap sent = v == sink ? frame.limit : frame.sent;
            frames_.pop_back();
            if (frames_.empty()) return sent;
            auto& parent = frames_.back(); auto& edge = graph_[parent.vertex][next_[parent.vertex]];
            edge.residual -= sent; graph_[edge.to][edge.reverse].residual += sent;
            parent.sent += sent;
            if (edge.residual == 0 || level_[edge.to] == -1) ++next_[parent.vertex];
        }
    }
    Cap unit_flow(int source, int sink) {
        std::fill(level_.begin(), level_.end(), -1);
        int front = 0, tail = 0;
        queue_[tail++] = source;
        level_[source] = source;
        while (front != tail) {
            const int v = queue_[front++];
            for (int index = 0; index != static_cast<int>(graph_[v].size()); ++index) {
                const auto& edge = graph_[v][index];
                const int u = edge.to;
                if (edge.residual == 0 || level_[u] != -1) continue;
                level_[u] = v;
                next_[u] = index;
                if (u == sink) {
                    for (int to = sink; to != source; to = level_[to]) {
                        auto& arc = graph_[level_[to]][next_[to]];
                        --arc.residual;
                        ++graph_[to][arc.reverse].residual;
                    }
                    return Cap{1};
                }
                queue_[tail++] = u;
            }
        }
        return Cap{0};
    }
    Cap preflow(int source, int sink, Cap limit) {
        const std::size_t n = graph_.size();
        const std::uint32_t virtual_height = static_cast<std::uint32_t>(n + 1);
        constexpr std::uint32_t unseen = std::numeric_limits<std::uint32_t>::max();
        // 確保失敗時は、残余を変更する前に退出する。
        std::vector<std::uint32_t> height(n);
        std::vector<Cap> excess(n, Cap{0});
        std::vector<int> cursor(n, 0), fifo(n);
        std::vector<unsigned char> active(n, 0);
        std::size_t front = 0, back = 0, count = 0;
        auto enqueue = [&](int v) {
            if (v == sink || active[v] || excess[v] == 0) return;
            active[v] = 1;
            fifo[back] = v;
            if (++back == n) back = 0;
            ++count;
        };
        auto relabel_all = [&] {
            // activeは再構築中だけBFSの訪問印として使う。
            std::fill(active.begin(), active.end(), 0);
            auto assign_height = [&](int v, std::uint32_t value) {
                if (height[v] != value) cursor[v] = 0;
                height[v] = value;
                active[v] = 1;
            };
            int head = 0, tail = 0;
            queue_[tail++] = sink;
            assign_height(sink, 0);
            while (head != tail) {
                const int v = queue_[head++];
                for (const auto& edge : graph_[v]) {
                    const int u = edge.to;
                    if (!active[u] && graph_[u][edge.reverse].residual != 0) {
                        assign_height(u, height[v] + 1);
                        queue_[tail++] = u;
                    }
                }
            }
            // tへ届かない余剰も、s経由で仮想始点へ戻して流量保存を回復する。
            if (!active[source]) {
                head = tail = 0;
                assign_height(source, virtual_height + 1);
                queue_[tail++] = source;
                while (head != tail) {
                    const int v = queue_[head++];
                    for (const auto& edge : graph_[v]) {
                        const int u = edge.to;
                        if (!active[u] && graph_[u][edge.reverse].residual != 0) {
                            assign_height(u, height[v] + 1);
                            queue_[tail++] = u;
                        }
                    }
                }
            }
            for (std::size_t v = 0; v < n; ++v) {
                if (!active[v] && height[v] != unseen) {
                    height[v] = unseen;
                    cursor[v] = 0;
                }
                active[v] = 0;
            }
            front = back = count = 0;
            for (int v = 0; v < static_cast<int>(n); ++v) enqueue(v);
        };
        excess[source] = limit;
        relabel_all();
        std::uint64_t work = 0;
        const std::uint64_t budget = static_cast<std::uint64_t>(positions_.size()) + n;
        while (count != 0) {
            const int v = fifo[front];
            if (++front == n) front = 0;
            --count;
            active[v] = 0;
            for (;;) {
                if (v == source && height[v] >= virtual_height + 1) {
                    // 容量limitの暗黙の逆辺で、未使用予算を返す。
                    excess[v] = 0;
                    break;
                }
                const int degree = static_cast<int>(graph_[v].size());
                int& index = cursor[v];
                while (index != degree) {
                    auto& edge = graph_[v][index];
                    ++work;
                    if (edge.residual != 0 && height[edge.to] != unseen &&
                        height[v] == height[edge.to] + 1) {
                        const Cap amount = std::min(excess[v], edge.residual);
                        edge.residual -= amount;
                        graph_[edge.to][edge.reverse].residual += amount;
                        excess[v] -= amount;
                        excess[edge.to] += amount;
                        enqueue(edge.to);
                        if (excess[v] == 0) break;
                    }
                    ++index;
                }
                if (excess[v] == 0) break;
                std::uint32_t best = v == source ? virtual_height : unseen;
                for (const auto& edge : graph_[v]) {
                    if (edge.residual != 0) best = std::min(best, height[edge.to]);
                }
                work += graph_[v].size();
                // 正の余剰には、流した経路の逆辺で仮想始点へ戻る経路が必ずある。
                height[v] = best + 1;
                index = 0;
                if (work >= budget) break;
            }
            if (work >= budget) {
                relabel_all();
                work = 0;
            }
        }
        return excess[sink];
    }
public:
    mf_graph() = default;
    explicit mf_graph(int n) : graph_(checked_size(n)), level_(graph_.size()), next_(graph_.size()), queue_(graph_.size()) {}
    mf_graph(const mf_graph&) = default;
    mf_graph(mf_graph&& other) noexcept { swap(other); }
    mf_graph& operator=(mf_graph other) noexcept { swap(other); return *this; }
    void swap(mf_graph& other) noexcept {
        graph_.swap(other.graph_); positions_.swap(other.positions_); level_.swap(other.level_);
        next_.swap(other.next_); queue_.swap(other.queue_); frames_.swap(other.frames_);
    }
    void reset(int n = 0) { mf_graph replacement(n); swap(replacement); }
    int size() const noexcept { return static_cast<int>(graph_.size()); }
    std::size_t edge_count() const noexcept { return positions_.size(); }
    void reserve_edges(std::size_t count) {
        if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("mf_graph: too many edges");
        positions_.reserve(count);
    }
    int add_edge(int from, int to, Cap capacity) {
#ifndef SKIP_RUNTIME_CHECKS
        vertex(from); vertex(to); nonnegative(capacity);
#endif
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckMaxFlowAppendCapacity(
            positions_.size(), graph_[from].size(), graph_[to].size(), from == to);
        const int forward = static_cast<int>(graph_[from].size());
        const int backward = static_cast<int>(graph_[to].size()) + static_cast<int>(from == to);
        graph_[from].push_back({to, backward, capacity});
        try { graph_[to].push_back({from, forward, 0}); }
        catch (...) { graph_[from].pop_back(); throw; }
        try { positions_.emplace_back(from, forward); }
        catch (...) { graph_[to].pop_back(); graph_[from].pop_back(); throw; }
        return static_cast<int>(positions_.size()) - 1;
    }
    edge get_edge(int id) { return std::as_const(*this).get_edge(id); }
    edge get_edge(int id) const {
#ifndef SKIP_RUNTIME_CHECKS
        edge_id(id);
#endif
        const auto [from, position] = positions_[id]; const auto& edge = graph_[from][position];
        const Cap sent = graph_[edge.to][edge.reverse].residual;
        return {from, edge.to, static_cast<Cap>(edge.residual + sent), sent};
    }
    std::vector<edge> edges() { return std::as_const(*this).edges(); }
    std::vector<edge> edges() const {
        std::vector<edge> result; result.reserve(positions_.size());
        for (int id = 0; id < static_cast<int>(positions_.size()); ++id) result.push_back(get_edge(id));
        return result;
    }
    void change_edge(int id, Cap capacity, Cap sent) {
#ifndef SKIP_RUNTIME_CHECKS
        edge_id(id); nonnegative(capacity); nonnegative(sent);
        if (sent > capacity) throw std::invalid_argument("mf_graph: flow exceeds capacity");
#endif
        const auto [from, position] = positions_[id]; auto& edge = graph_[from][position];
        edge.residual = capacity - sent; graph_[edge.to][edge.reverse].residual = sent;
    }
    Cap flow(int source, int sink) { return flow(source, sink, std::numeric_limits<Cap>::max()); }
    Cap flow(int source, int sink, Cap limit) {
#ifndef SKIP_RUNTIME_CHECKS
        vertex(source); vertex(sink); nonnegative(limit);
#endif
        if (source == sink || limit == 0) return 0;
        if (limit == Cap{1}) return unit_flow(source, sink);
        frames_.reserve(graph_.size());
        Cap result = 0;
        int phases = 0;
        // 最大頂点数では高さの上限と未発見印が同じ値になるため、Dinicを続ける。
        bool allow_preflow = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::MaxFlowCanUsePreflow(graph_.size());
        while (result != limit && levels(source, sink)) {
            if (allow_preflow && phases >= 4) {
                try { return result + preflow(source, sink, static_cast<Cap>(limit - result)); }
                catch (const std::bad_alloc&) { allow_preflow = false; }
            }
            std::fill(next_.begin(), next_.end(), 0);
            result += blocking_flow(source, sink, static_cast<Cap>(limit - result));
            ++phases;
        }
        return result;
    }
    std::vector<bool> min_cut(int source) { return std::as_const(*this).min_cut(source); }
    std::vector<bool> min_cut(int source) const {
#ifndef SKIP_RUNTIME_CHECKS
        vertex(source);
#endif
        std::vector<bool> reachable(graph_.size()); std::vector<int> queue;
        queue.reserve(graph_.size()); queue.push_back(source); reachable[source] = true;
        for (std::size_t front = 0; front < queue.size(); ++front) {
            const int v = queue[front];
            for (const auto& edge : graph_[v]) if (edge.residual != 0 && !reachable[edge.to]) {
                reachable[edge.to] = true; queue.push_back(edge.to);
            }
        }
        return reachable;
    }
};
#endif

#ifndef SIRKIT_REACHABILITY_CHAIN_COVER_DEFINED
#define SIRKIT_REACHABILITY_CHAIN_COVER_DEFINED

namespace detail
{
inline void check_chain_cover_network_size(std::size_t components)
{
    constexpr auto kMaximum = static_cast<std::size_t>((std::numeric_limits<int>::max() - 2) / 2);
    if (components > kMaximum)
    {
        throw std::length_error("reachability chain cover: too many components");
    }
}
} // namespace detail

inline std::vector<std::vector<int>> minimum_reachability_chain_cover(
    int n, std::span<const std::pair<int, int>> edges)
{
    scc_graph graph(n);
    graph.reserve_edges(edges.size());
    for (const auto &[u, v] : edges)
    {
        graph.add_edge(u, v);
    }
    const auto groups = graph.scc();
    detail::check_chain_cover_network_size(groups.size());
    const int count = static_cast<int>(groups.size());
    const auto dag = graph.condensation();

    // 各成分だけを通る流量1の歩道から始め、残余グラフで歩道同士をつなぐ。
    const int cancel_from = 2 * count, cancel_to = cancel_from + 1;
    mf_graph<int> residual(cancel_to + 1);
    std::vector<int> cancel_starts(count), cancel_ends(count);
    struct Transit
    {
        int to, id, remaining = 0;
    };
    std::vector<std::vector<Transit>> outgoing(count);
    for (int v = 0; v < count; ++v)
    {
        cancel_starts[v] = residual.add_edge(2 * v, cancel_to, 1);
        cancel_ends[v] = residual.add_edge(cancel_from, 2 * v + 1, 1);
        residual.add_edge(2 * v, 2 * v + 1, count);
        outgoing[v].reserve(dag[v].size());
        for (const int u : dag[v])
        {
            const int flow_edge = residual.add_edge(2 * v + 1, 2 * u, count);
            outgoing[v].push_back({u, flow_edge});
        }
    }
    const int saved = residual.flow(cancel_from, cancel_to);
    std::vector<bool> can_end(count), assigned(count);
    for (int v = 0; v < count; ++v)
    {
        can_end[v] = residual.get_edge(cancel_ends[v]).flow == 0;
        for (auto &transit : outgoing[v])
        {
            transit.remaining = residual.get_edge(transit.id).flow;
        }
    }

    std::vector<std::vector<int>> chains;
    chains.reserve(count - saved);
    std::vector<std::size_t> cursor(count);
    for (int start = 0; start < count; ++start)
    {
        if (residual.get_edge(cancel_starts[start]).flow != 0)
        {
            continue;
        }
        auto &chain = chains.emplace_back();
        int v = start;
        for (;;)
        {
            if (!assigned[v])
            {
                chain.insert(chain.end(), groups[v].begin(), groups[v].end());
                assigned[v] = true;
            }
            if (can_end[v])
            {
                can_end[v] = false;
                break;
            }
            while (outgoing[v][cursor[v]].remaining == 0)
            {
                ++cursor[v];
            }
            auto &next = outgoing[v][cursor[v]];
            --next.remaining;
            v = next.to;
        }
    }
    return chains;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
