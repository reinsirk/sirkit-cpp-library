// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Walk_Weight_Ranges.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/walk_weight_ranges.md
//
// API一覧
// WalkWeightEdge{from,to,weight}: 0始まりの端点とint32_tの符号付き辺重み。
// walk_weight_ranges(n,edges,source): 始点から各頂点へ着く歩道の最小・最大重みを調べる。
//   頂点・辺を再訪でき、始点での空の歩道も許す。O(n+m)時間・領域。
// WalkWeightRange{reachable,unbounded,minimum,maximum}: 到達可否、一方以上の非有界性、最小・最大値。
//   非有界または到達不能ならminimumとmaximumは0。有限な値はint64_t。


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
#if defined(__GNUC__) || defined(__clang__)
[[gnu::noinline]]
#endif
inline bool outdegrees_all_one(const std::vector<Index>& degree) {
    for(auto it=degree.begin()+1;it!=degree.end();++it)if(*it!=1)return false;
    return true;
}

// 出次数が全て1なら、閉路と流入木を反復的に分解する。
#if defined(__GNUC__) || defined(__clang__)
[[gnu::noinline]]
#endif
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
    }
    void reset(int n = 0) { scc_graph replacement(n); swap(replacement); }
    int size() const noexcept { return n_; }
    std::size_t edge_count() const noexcept { return edges_.size(); }
    void reserve_edges(std::size_t count) { edges_.reserve(count); }
    void add_edge(int u, int v) {
        edges_.emplace_back(u, v);
    }
    int build() {
        auto result = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::directed_component_ids(n_, edges_);
        ids_.swap(result.second); count_ = result.first;
        return count_;
    }
    int component_count() const {
        return count_;
    }
    int id(int v) const {
        return ids_[v];
    }
    const std::vector<int>& ids() const {
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

#ifndef SIRKIT_WALK_WEIGHT_RANGES_DEFINED
#define SIRKIT_WALK_WEIGHT_RANGES_DEFINED

struct WalkWeightEdge
{
    int from, to;
    std::int32_t weight;
};

struct WalkWeightRange
{
    bool reachable = false;
    bool unbounded = false;
    std::int64_t minimum = 0;
    std::int64_t maximum = 0;
};

inline std::vector<WalkWeightRange> walk_weight_ranges(int n, std::span<const WalkWeightEdge> edges, int source)
{
    scc_graph graph(n);
    graph.reserve_edges(edges.size());
    std::vector<std::vector<std::pair<int, std::int32_t>>> adjacent(n);
    for (const auto &[u, v, weight] : edges)
    {
        graph.add_edge(u, v);
        adjacent[u].emplace_back(v, weight);
    }
    const auto groups = graph.scc();
    const auto &id = graph.ids();

    // 成分内で辺の重みをポテンシャル差として表せない場合、非零の閉じた歩道がある。
    std::vector<bool> nonzero_cycle(groups.size()), entered(n);
    std::vector<std::int64_t> potential(n);
    std::vector<int> pending;
    for (std::size_t component = 0; component < groups.size(); ++component)
    {
        const int root = groups[component].front();
        entered[root] = true;
        pending.push_back(root);
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const int u = pending[i];
            for (const auto &[v, weight] : adjacent[u])
            {
                if (id[v] != static_cast<int>(component))
                {
                    continue;
                }
                const auto candidate = potential[u] + weight;
                if (!entered[v])
                {
                    entered[v] = true;
                    potential[v] = candidate;
                    pending.push_back(v);
                }
                else if (candidate != potential[v])
                {
                    nonzero_cycle[component] = true;
                }
            }
        }
        pending.clear();
    }

    // SCCの番号はトポロジカル順。代表点の最小・最大値を成分間の辺で更新する。
    std::vector<WalkWeightRange> states(groups.size());
    states[id[source]] = {true, false, -potential[source], -potential[source]};
    for (std::size_t component = 0; component < groups.size(); ++component)
    {
        auto &range = states[component];
        if (!range.reachable)
        {
            continue;
        }
        if (nonzero_cycle[component])
        {
            range = {true, true, 0, 0};
        }
        for (const int u : groups[component])
        {
            for (const auto &[v, weight] : adjacent[u])
            {
                if (id[v] == static_cast<int>(component))
                {
                    continue;
                }
                auto &target = states[id[v]];
                if (range.unbounded)
                {
                    target = {true, true, 0, 0};
                }
                else if (!target.unbounded)
                {
                    const auto shift = potential[u] + weight - potential[v];
                    const auto low = range.minimum + shift;
                    const auto high = range.maximum + shift;
                    if (!target.reachable)
                    {
                        target = {true, false, low, high};
                    }
                    else
                    {
                        target.minimum = std::min(target.minimum, low);
                        target.maximum = std::max(target.maximum, high);
                    }
                }
            }
        }
    }

    std::vector<WalkWeightRange> result(n);
    for (int vertex = 0; vertex < n; ++vertex)
    {
        result[vertex] = states[id[vertex]];
        if (result[vertex].reachable && !result[vertex].unbounded)
        {
            result[vertex].minimum += potential[vertex];
            result[vertex].maximum += potential[vertex];
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
