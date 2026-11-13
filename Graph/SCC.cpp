// SPDX-License-Identifier: CC0-1.0
// 参考（問題選定・強連結成分の考え方）: https://nyaannyaan.github.io/library/graph/strongly-connected-components.hpp
// 参考（API仕様）: https://atcoder.github.io/ac-library/production/document_ja/scc.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/scc.md
//
// API一覧
// scc_graph() / scc_graph(n): 空／n頂点の有向グラフ。頂点番号[0,n)。O(1)準備。
// scc_graph(g): Graphから構築・分解。自己辺・多重辺可。O(n+m)時間・領域。
// add_edge(u,v): 有向辺追加。償却O(1)。分解結果を無効化する。reserve_edges(m): m辺分を確保。
// scc(): 分解し、トポロジカル順のvector<vector<int>>を返す。群内は頂点番号の昇順。O(n+m)時間・領域。
// build(): 頂点群を作らず分解し、成分数を返す追加API。O(n+m)時間・領域。
// size()/edge_count(): 頂点数/辺数。O(1)。component_count()/id(v)/ids(): 成分数/番号/番号配列へのconst参照。O(1)。
// 成分番号は縮約DAGのトポロジカル順。異成分の辺u→vではid(u)<id(v)。結果の照会はscc/build後に行う。
// condensation(unique=true): 縮約Graph。自己辺は除き、既定では多重辺も除く。O(n+m)時間・追加領域。
// reset(n=0): 空のn頂点へ戻す。swap(other): O(1)交換。コピーは独立所有、ムーブ元は空の未分解グラフ。
// 入力検査はSKIP_RUNTIME_CHECKSで除く。ids()の参照は再scc/build・代入・破棄で無効になる。

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_UNWEIGHTED_GRAPH_DEFINED
#define SIRKIT_UNWEIGHTED_GRAPH_DEFINED
using Graph = std::vector<std::vector<int>>;
#endif

#ifndef SIRKIT_SCC_DETAIL_DEFINED
#define SIRKIT_SCC_DETAIL_DEFINED
namespace reinsirk::detail {
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
inline std::pair<int, std::vector<int>> directed_component_ids(
        int n, const std::vector<std::pair<int, int>>& edges) {
    // 無辺では各頂点が1成分。従来と同じ逆番号で返す。
    if (edges.empty()) {
        std::vector<int> ids(n);
        for (int v = 0; v < n; ++v) ids[v] = n - 1 - v;
        return {n, std::move(ids)};
    }
    // 辺数が収まる場合だけCSRの添字を32bitにする。
    if (edges.size() <= std::numeric_limits<std::uint32_t>::max())
        return directed_component_ids_index<std::uint32_t>(n, edges);
    return directed_component_ids_index<std::size_t>(n, edges);
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
    explicit scc_graph(const Graph& g) {
        if (g.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("SCC: too many vertices");
        n_ = static_cast<int>(g.size());
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
        auto result = reinsirk::detail::directed_component_ids(n_, edges_);
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
