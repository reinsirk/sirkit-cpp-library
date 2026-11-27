// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Incremental_SCC.cpp
// 参考（オフライン版の時刻分割・縮約）: https://doi.org/10.4230/LIPIcs.SWAT.2026.17
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/incremental_scc.md
//
// API一覧
// incremental_scc_merges(n,edges): 空の有向グラフに全辺を入力順で追加したときのSCC併合を時刻順に返す。
// IncrementalSccMerge{time,leader,absorbed}: time本目の追加後に、absorbedを代表とする成分をleaderに併合する。
// 代表の初期値は自分自身。同じ時刻のイベントは結果の順に適用する。頂点番号は[0,n)、時刻0では併合なし。
// 自己辺・重複辺可。時刻分割と縮約でO(n+m log(m+1))時間、O(n+m)領域。
// 1時点の成分分解にはscc_graph、全追加が既知の場合の併合列にはincremental_scc_mergesを使う。
//
// [同梱SCC]
// 参考（問題選定・強連結成分の考え方）: https://nyaannyaan.github.io/library/graph/strongly-connected-components.hpp
// 参考（API仕様）: https://atcoder.github.io/ac-library/production/document_ja/scc.html
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
//
// [同梱UnionFind]
// Original Source: https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/union-find
// [UnionFind]
// UnionFind(n=0): n要素を独立成分として構築。O(n+1)時間・領域、n<=INT_MAX。
// root(i)/leader(i): 代表、unite(a,b): 結合（戻り値なし）、merge(a,b): 新規結合ならtrue。償却O(α(n+1))。
// issame(a,b)/same(a,b): 同一成分か、size(i): 成分サイズ。償却O(α(n+1))。探索時に経路を圧縮する。
// size()/empty()/component_count(): 全要素数/空か/成分数、O(1)。groups(): 頂点昇順の成分一覧、O(1+n α(n+1))償却。
// reset(n)/reset(): n要素/現在の要素数へ初期化。容量を再利用、O(旧n+n+1)上界。確保失敗時は元を保つ。
// clear(): 空へ戻して容量保持、O(旧n+1)上界、noexcept。swap(other): 内容交換、O(1)、noexcept。
// コピーはO(n+1)で独立、移動元は空。自己代入・自己ムーブは内容を保つ。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <span>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
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


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <algorithm>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <numeric>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_UNION_FIND_DEFINED
#define SIRKIT_UNION_FIND_DEFINED
class UnionFind {
    std::vector<int> param;
    int components_ = 0;
    static std::size_t checked_size(std::size_t n) {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("UnionFind: too many vertices");
        return n;
    }
    void check(int i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(i) >= param.size())
            throw std::out_of_range("UnionFind: vertex");
#else
        (void)i;
#endif
    }
    int leader_unchecked(int i) {
        while (param[i] >= 0) {
            const int p = param[i];
            if (param[p] < 0) return p;
            i = param[i] = param[p];
        }
        return i;
    }
#if defined(__clang__)
    // 単一成分の連続処理を分離し、通常の成分列挙のコード生成を保つ。
    void list_single_component(std::vector<int>& vertices) {
        const int root = leader_unchecked(0);
        const int negative_size = param[root];
        std::fill(param.begin(), param.end(), root);
        param[root] = negative_size;
        std::iota(vertices.begin(), vertices.end(), 0);
    }
#endif
public:
    explicit UnionFind(std::size_t n = 0)
        : param(checked_size(n), -1), components_(static_cast<int>(n)) {}
    UnionFind(const UnionFind&) = default;
    UnionFind& operator=(const UnionFind&) = default;
    UnionFind(UnionFind&& other) noexcept
        : param(std::move(other.param)), components_(std::exchange(other.components_, 0)) { other.param.clear(); }
    UnionFind& operator=(UnionFind&& other) noexcept {
        if (this != &other) {
            param = std::move(other.param);
            components_ = std::exchange(other.components_, 0); other.param.clear();
        }
        return *this;
    }
    void reset(std::size_t n) {
        const auto count = checked_size(n);
        param.reserve(count);
        param.clear(); param.resize(count, -1);
        components_ = static_cast<int>(count);
    }
    void reset() { reset(param.size()); }
    void clear() noexcept { param.clear(); components_ = 0; }
    bool empty() const noexcept { return param.empty(); }
    void swap(UnionFind& other) noexcept {
        param.swap(other.param); std::swap(components_, other.components_);
    }
    friend void swap(UnionFind& left, UnionFind& right) noexcept { left.swap(right); }
    int root(int i) { check(i); return leader_unchecked(i); }
    int leader(int i) { return root(i); }
    bool merge(int a, int b) {
        check(a); check(b);
#if defined(__GNUC__) && !defined(__clang__)
        // 経路圧縮は根の負サイズを変えないので、第二探索後も再利用できる。
        int sa = param[a];
        while (sa >= 0) {
            const int next = param[sa];
            if (next < 0) { a = sa; sa = next; break; }
            a = param[a] = next; sa = param[a];
        }
        // 根自身・根直下なら第二探索の書戻しは発生しない。
        if (a == b) return false;
        int sb = param[b];
        while (sb >= 0) {
            if (sb == a) return false;
            const int next = param[sb];
            if (next < 0) { b = sb; sb = next; break; }
            b = param[b] = next; sb = param[b];
        }
        if (a == b) return false;
        if (sa > sb) std::swap(a, b);
        param[a] = sa + sb; param[b] = a; --components_;
#else
        a = leader_unchecked(a); b = leader_unchecked(b);
        if (a == b) return false;
        if (param[a] > param[b]) std::swap(a, b);
        param[a] += param[b]; param[b] = a; --components_;
#endif
        return true;
    }
    void unite(int a, int b) { (void)merge(a, b); }
    bool same(int a, int b) { check(a); check(b); return leader_unchecked(a) == leader_unchecked(b); }
    bool issame(int a, int b) { return same(a, b); }
    int size(int i) { return -param[root(i)]; }
    int size() const noexcept { return static_cast<int>(param.size()); }
    int component_count() const noexcept { return components_; }
    std::vector<std::vector<int>> groups() {
        std::vector<std::vector<int>> result;
        result.reserve(static_cast<std::size_t>(components_));
        if (components_ == size()) {
            for (int v = 0; v < size(); ++v) result.emplace_back(std::size_t{1}, v);
            return result;
        }
        if (components_ == 1) {
#if defined(__clang__)
            result.emplace_back(param.size());
            list_single_component(result[0]);
#else
            const int root = leader_unchecked(0);
            result.emplace_back(param.size());
            const int negative_size = param[root];
            std::fill(param.begin(), param.end(), root);
            param[root] = negative_size;
            std::iota(result[0].begin(), result[0].end(), 0);
#endif
            return result;
        }
#if defined(__clang__)
        std::vector<int> leaders(param.size()), counts(param.size(), 0);
        for (int i = 0; i < size(); ++i) ++counts[leaders[i] = leader_unchecked(i)];
        for (int i = 0; i < size(); ++i) if (counts[i]) {
            const int count = counts[i];
            counts[i] = static_cast<int>(result.size());
            result.emplace_back(); result.back().reserve(count);
        }
        for (int i = 0; i < size(); ++i) result[counts[leaders[i]]].push_back(i);
#else
        std::vector<int> slot(param.size());
        for (int i = 0; i < size(); ++i) if (param[i] < 0) {
            slot[i] = static_cast<int>(result.size());
            result.emplace_back(); result.back().reserve(static_cast<std::size_t>(-param[i]));
        }
        for (int i = 0; i < size(); ++i) result[slot[leader_unchecked(i)]].push_back(i);
#endif
        return result;
    }
};
#endif

#ifndef SIRKIT_INCREMENTAL_SCC_DEFINED
#define SIRKIT_INCREMENTAL_SCC_DEFINED

struct IncrementalSccMerge
{
    std::size_t time;
    int leader;
    int absorbed;
};

namespace detail
{
struct IncrementalSccEdge
{
    std::size_t inserted;
    int from;
    int to;
};

class IncrementalSccSolver
{
    std::vector<int> index_, vertices_, representative_;
    std::vector<std::pair<int, int>> prefix_;
    UnionFind groups_;
    std::vector<IncrementalSccMerge> events_;

    int index(int vertex)
    {
        auto &position = index_[vertex];
        if (position == -1)
        {
            position = static_cast<int>(vertices_.size());
            vertices_.push_back(vertex);
        }
        return position;
    }

    auto label_prefix(std::span<const IncrementalSccEdge> edges, std::size_t time)
        -> std::pair<int, std::vector<int>>
    {
        prefix_.clear();
        for (const auto &[inserted, from, to] : edges)
        {
            const int left = index(from);
            const int right = index(to);
            if (inserted <= time)
            {
                prefix_.emplace_back(left, right);
            }
        }
        return directed_component_ids(static_cast<int>(vertices_.size()), prefix_);
    }

    void clear_indices()
    {
        for (int v : vertices_)
        {
            index_[v] = -1;
        }
        vertices_.clear();
    }

    std::pair<std::size_t, std::size_t> split(std::span<IncrementalSccEdge> edges, std::size_t mid)
    {
        const auto [count, ids] = label_prefix(edges, mid);
        if (static_cast<std::size_t>(count) == vertices_.size())
        {
            clear_indices();
            return {0, edges.size()};
        }

        const auto same = [&](const IncrementalSccEdge &edge)
        {
            return ids[index_[edge.from]] == ids[index_[edge.to]];
        };
        // 左に残すのは、midですでに追加されている成分内の辺だけ。
        const auto middle = std::partition(edges.begin(), edges.end(), [&](const auto &edge)
                                           {
                                               return edge.inserted <= mid && same(edge);
                                           });
        // midの成分内にあって、追加がこれからの辺は以後の成分を変えない。
        const auto kept_end = std::remove_if(middle, edges.end(), same);

        representative_.resize(count);
        for (std::size_t i = 0; i < vertices_.size(); ++i)
        {
            representative_[ids[i]] = vertices_[i];
        }
        // 右へ送る辺はmidの成分代表を端点に使う。
        for (auto it = middle; it != kept_end; ++it)
        {
            it->from = representative_[ids[index_[it->from]]];
            it->to = representative_[ids[index_[it->to]]];
        }
        clear_indices();
        return {static_cast<std::size_t>(middle - edges.begin()), static_cast<std::size_t>(kept_end - edges.begin())};
    }

    void solve(std::span<IncrementalSccEdge> edges, std::size_t first, std::size_t last)
    {
        if (edges.empty())
        {
            return;
        }
        if (first == last)
        {
            for (const auto &edge : edges)
            {
                const int from = groups_.root(edge.from);
                const int to = groups_.root(edge.to);
                if (groups_.merge(from, to))
                {
                    const int leader = groups_.root(from);
                    events_.push_back({first, leader, leader == from ? to : from});
                }
            }
            return;
        }

        const auto mid = first + (last - first) / 2;
        const auto [left, kept] = split(edges, mid);
        solve(edges.first(left), first, mid);
        solve(edges.subspan(left, kept - left), mid + 1, last);
    }

public:
    explicit IncrementalSccSolver(int n)
        : index_(n, -1), groups_(n)
    {
    }

    std::vector<IncrementalSccMerge> run(std::span<IncrementalSccEdge> edges, std::size_t last)
    {
        // 最終的に異なる成分を結ぶ辺は、どの時刻の閉路にも入らない。
        const auto [count, final] = label_prefix(edges, last);
        static_cast<void>(count);
        const auto end = std::remove_if(edges.begin(), edges.end(), [&](const auto &edge)
                                        {
                                            return final[index_[edge.from]] != final[index_[edge.to]];
                                        });
        const auto kept = static_cast<std::size_t>(end - edges.begin());
        clear_indices();
        events_.reserve(std::min(kept, index_.size() - 1));
        solve(edges.first(kept), 1, last);
        return std::move(events_);
    }
};
} // namespace detail

inline std::vector<IncrementalSccMerge> incremental_scc_merges(
    int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("incremental_scc_merges: negative vertex count");
    }
    std::vector<detail::IncrementalSccEdge> work;
    work.reserve(edges.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &[from, to] = edges[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("incremental_scc_merges: edge endpoint out of range");
        }
#endif
        if (from != to)
        {
            work.push_back({i + 1, from, to});
        }
    }
    if (work.empty())
    {
        return {};
    }
    detail::IncrementalSccSolver solver(n);
    return solver.run(work, edges.size());
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
