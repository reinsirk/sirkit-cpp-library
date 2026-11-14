// SPDX-License-Identifier: CC0-1.0
// 参考（API設計・CC0）: https://github.com/NyaanNyaan/library/blob/2e713880f0d5041d794e0f09c7f31c1826ba0ccf/tree/heavy-light-decomposition.hpp
// 参考元の提供条件: https://github.com/NyaanNyaan/library/blob/2e713880f0d5041d794e0f09c7f31c1826ba0ccf/LICENSE
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/heavy_light_decomposition.md
//
// API一覧
// [共通型] Graphはvector<vector<int>>。HeavyLightDecomposition::Intervalは{int left,right;bool reversed;}。
// [値を持つHLD: HeavyLightValues<Range,Ordered=false>]
// value_typeはBITのT/Segtree・LazyのS。HeavyLightValues()は空。コピー構築/代入は独立、移動構築はO(1)。
// operator=(other)は自身への参照を返す。代入/buildは旧状態の破棄費用も加わる。
// with_values<Range,Ordered>(values): values[v]を頂点vの値とする長さNのvectorから、分解と集計器を独立所有するHLDを返す。
// 元の値配列とは更新を共有しない。get/set/add/applyのvは頂点番号で、index(v)への変換は不要。
// 一時HLDなら分解を移動、既存HLDならコピー。RangeはBIT/ACL形式Segtree/Lazy。値の型はRangeから推論。
// HeavyLightValues(g,values,root=0)/build(g,values,root=0): 分解と値を同時に構築/再構築。失敗時は元を保持。
// HeavyLightValues(layout,values): 分解をコピー、std::move(layout)なら移動。移動後の失敗では元分解が空の場合あり。
// 構築は分解O(N+M)+集計器のvector構築。vector構築なしのBITはN回add。所有状態を1回確保する。
// get(v)/set(v,x)/add(v,x): 頂点番号で取得/代入/加算。addは対応BITのみ。通常の集計器ではO(log N)。
// apply(v,f)/path_apply(u,v,f,edges=false)/subtree_apply(v,f,edges=false): 点/経路/部分木へ作用。
// path_prod(u,v,edges=false)/subtree_prod(v,edges=false): 所有集計器で経路/部分木を集計。通常O(log²N)/O(log N)。
// Ordered=falseは和・最小値などの可換演算用。trueは文字列連結などをu→vの順に集計。領域と更新は集計器2本分。
// trueは値の並び順を守る設定で、個々の辺の値を逆元へ変える機能ではない。部分木の積はDFS添字昇順。
// 辺モードは子vに親辺の値を置き、各根には0/e()を置く。初期値はN個必要。edges=trueでLCA/部分木の指定頂点を除く。
// path_prod(v,v)は頂点の値、path_prod(v,v,true)や葉の辺部分木は空区間の単位元。空区間の更新は何もしない。
// values(): 元の頂点順へ復元。Range.valuesがあればO(N)の並べ直し、なければN回点照会。
// clear()/swap(other): 空へ/交換。コピーは独立、move元は空。build/コピー失敗時は元を保持。
// 分解情報・reorder/restore/for_each_path/pathも同名で取得可能。対応applyを持つRangeでのみ作用APIを使用できる。
// 所有型にInterval型別名・with_values・外部集計器を受けるオーバーロードはない。pathの要素型は上記Interval。
// 値の照会も非constの所有HLDから呼ぶ。根を変えて辺の値をbuildする場合は、新しい親子対応で値を配置し直す。
// 値の更新中の例外では変更は巻き戻さない。Ordered=trueでは例外後にclear/buildで再構築すること。
// [分解だけを持つHLD: HeavyLightDecomposition]
// HeavyLightDecomposition()は空、コピー構築はO(N)、移動構築はO(1)。operator=(other)は自身への参照を返す。
// コピー・移動代入とbuildは旧状態の破棄費用も加わる。
// HeavyLightDecomposition(g,root=0)/build(g,root=0): 無向森を反復DFSで分解。O(n+m)時間・領域。
// 頂点番号[0,n)。隣接リストの対称性は前提。rootの木から、残りは最小未訪問頂点から構築する。
// 失敗時は元を保持。空のgにはroot=0。閉路・自己辺・多重辺は検査有効時に例外。
// size()/empty()/root()/component_count(): 頂点数/空か/指定根（空なら-1）/木の数。O(1)。
// parent(v)/depth(v)/component(v)/root_of(v): 親（根は-1）/深さ/木の番号/木の根。O(1)。
// index(v)/vertex_at(i)/order(): heavy優先DFS添字/対応頂点/順序配列へのconst参照。O(1)。
// same_component(u,v)/is_ancestor(u,v): 同じ木か/祖先か（自身を含む）。O(1)。
// subtree_range(v,edges=false): 部分木の半開区間。edges=trueは根vを除き子側の頂点で辺を表す。O(1)。
// subtree_size(v): 部分木の頂点数。O(1)。lca(u,v)/distance(u,v): LCA/辺数、異なる木は-1。O(log n)。
// kth_ancestor(v,k): k個上の頂点。k<0/根を越すと-1。jump(u,v,k): u→vのk番目、範囲外/異なる木は-1。O(log n)。
// for_each_path(u,v,fn,edges=false): 通行順にfn(left,right,reversed)を呼ぶ。fnの戻り値は無視。接続ならtrue、異なる木はfalse。
// reversed=trueはright-1からleftへ読む。区間は半開。edges=trueはLCAを除く。O(log n)、ヒープ確保なし。
// path(u,v,edges=false): 同じ順序のInterval{left,right,reversed}配列を返す。異なる木は空。O(log n)時間・領域。
// clear()/swap(other): 空に戻す/O(1)交換。コピーは独立所有、ムーブ元は空。検査はSKIP_RUNTIME_CHECKSで除く。
// reorder(values)/restore(ordered): 長さNのvectorをDFS添字順/元の頂点順へコピー。O(N)時間・領域。
// path_prod(u,v,bit,edges=false): sumを持つBITの和。単位元はT{}、整数は型幅で周回。O(log N)回照会。
// path_prod(u,v,seg,edges=false): ACL形式のSegtreeからop/eを推論した可換積。Segはconst可、Lazyは非const。
// path_prod(u,v,identity,query,op,edges=false): queryはprod/sum、2引数関数なら可換、3引数(l,r,reversed)なら向き対応。
// path_prod(u,v,seg,reverse,edges=false)/汎用版のop直後にreverse指定: 正しい逆順集約を返す非可換積。
// subtree_prod(v,query,edges=false): 部分木のDFS添字昇順の積。区間照会1回。非可換も可。
// path_apply(u,v,range,f,edges=false)/subtree_apply(v,range,f,edges=false): range.apply(l,r,f)で更新。O(log N)/1回。
// prod/applyのrangeは同じDFS添字順で用意する。path_prod/path_applyは同じ木が前提、検査時は別木を例外。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
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

#ifndef SIRKIT_HLD_QUERY_HELPERS_DEFINED
#define SIRKIT_HLD_QUERY_HELPERS_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class T, bool = std::is_integral_v<T> && !std::is_same_v<T,bool>>
struct HldArithmetic {
    static T add(const T& a,const T& b) { return a+b; }
    static T subtract(const T& a,const T& b) { return a-b; }
};
template<class T> struct HldArithmetic<T,true> {
    using U=std::make_unsigned_t<T>;
    static T add(T a,T b) { return static_cast<T>(static_cast<U>(a)+static_cast<U>(b)); }
    static T subtract(T a,T b) { return static_cast<T>(static_cast<U>(a)-static_cast<U>(b)); }
};
#if defined(__SIZEOF_INT128__)
template<> struct HldArithmetic<__int128,false> {
    using U=unsigned __int128;
    static __int128 add(__int128 a,__int128 b) { return static_cast<__int128>(static_cast<U>(a)+static_cast<U>(b)); }
    static __int128 subtract(__int128 a,__int128 b) { return static_cast<__int128>(static_cast<U>(a)-static_cast<U>(b)); }
};
template<> struct HldArithmetic<unsigned __int128,false> {
    using U=unsigned __int128;
    static U add(U a,U b) { return a+b; }
    static U subtract(U a,U b) { return a-b; }
};
#endif
template<class Q> concept HldQuery = requires(Q& q) { q.prod(0,0); } ||
    requires(Q& q) { q.sum(0,0); } || std::invocable<Q&,int,int,bool> || std::invocable<Q&,int,int>;
template<class Q> decltype(auto) hld_query(Q& q,int l,int r,bool reverse=false) {
    if constexpr (requires { q.prod(l,r); }) return q.prod(l,r);
    else if constexpr (requires { q.sum(l,r); }) return q.sum(l,r);
    else if constexpr (std::invocable<Q&,int,int,bool>) return std::invoke(q,l,r,reverse);
    else return std::invoke(q,l,r);
}
template<class Q> using HldQueryValue=std::remove_cvref_t<decltype(hld_query(std::declval<Q&>(),0,0))>;
template<class Q> inline constexpr bool hld_directional_query =
    !requires(Q& q) { q.prod(0,0); } && !requires(Q& q) { q.sum(0,0); } && std::invocable<Q&,int,int,bool>;
template<class I,class Q,class Op> concept HldProduct = HldQuery<Q> &&
    std::constructible_from<HldQueryValue<Q>,I> &&
    std::is_invocable_r_v<HldQueryValue<Q>,Op&,HldQueryValue<Q>,HldQueryValue<Q>>;
template<class I,class Q,class Op,class Reverse> concept HldReversedProduct = HldProduct<I,Q,Op> &&
    std::is_invocable_r_v<HldQueryValue<Q>,Reverse&,HldQueryValue<Q>> &&
    (!hld_directional_query<Q> || std::is_same_v<std::remove_cvref_t<Reverse>,std::identity>);
}
#endif

#ifndef SIRKIT_HEAVY_LIGHT_VALUES_FORWARD_DEFINED
#define SIRKIT_HEAVY_LIGHT_VALUES_FORWARD_DEFINED
template<class Range,bool Ordered=false> class HeavyLightValues;
#endif

#ifndef SIRKIT_HEAVY_LIGHT_DECOMPOSITION_DEFINED
#define SIRKIT_HEAVY_LIGHT_DECOMPOSITION_DEFINED
class HeavyLightDecomposition {
    std::vector<int> parent_,depth_,component_,roots_,index_,order_,subtree_,head_;
    int root_=-1;
    void check_vertex(int v) const {
#ifndef SKIP_RUNTIME_CHECKS
        if(v<0 || v>=size()) throw std::out_of_range("HLD: vertex");
#else
        (void)v;
#endif
    }
    void check_size(std::size_t n) const {
#ifndef SKIP_RUNTIME_CHECKS
        if(n!=order_.size()) throw std::invalid_argument("HLD: values size");
#else
        (void)n;
#endif
    }
    void construct(const Graph& graph,int root) {
        if(graph.size()>static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("HLD: int size overflow");
        const int n=static_cast<int>(graph.size());
#ifndef SKIP_RUNTIME_CHECKS
        if((n==0 && root!=0) || (n!=0 && (root<0 || root>=n))) throw std::out_of_range("HLD: root");
#endif
        if(n==0) return;
        root_=root;
        parent_.assign(n,-2); depth_.resize(n); component_.resize(n);
        index_.resize(n); subtree_.assign(n,1); head_.resize(n); order_.reserve(n);
        std::vector<int> pending,discovery,heavy(n,-1);
        pending.reserve(n); discovery.reserve(n);
        const auto grow=[&](int start) {
            const int id=static_cast<int>(roots_.size()); roots_.push_back(start);
            parent_[start]=-1;component_[start]=id;pending.push_back(start);
            while(!pending.empty()) {
                const int v=pending.back();pending.pop_back();discovery.push_back(v);
#ifndef SKIP_RUNTIME_CHECKS
                bool parent_seen=false;
#endif
                for(int u:graph[v]) {
#ifndef SKIP_RUNTIME_CHECKS
                    if(u<0 || u>=n) throw std::out_of_range("HLD: neighbor");
                    if(u==v) throw std::invalid_argument("HLD: self edge");
#endif
                    if(u==parent_[v]) {
#ifndef SKIP_RUNTIME_CHECKS
                        if(parent_seen) throw std::invalid_argument("HLD: multiple edge");
                        parent_seen=true;
#endif
                        continue;
                    }
#ifndef SKIP_RUNTIME_CHECKS
                    if(parent_[u]!=-2) throw std::invalid_argument("HLD: cycle or multiple edge");
#endif
                    parent_[u]=v;depth_[u]=depth_[v]+1;component_[u]=id;pending.push_back(u);
                }
            }
        };
        grow(root);
        for(int v=0;v<n;++v) if(parent_[v]==-2) grow(v);
        for(auto it=discovery.rbegin();it!=discovery.rend();++it) {
            const int v=*it,p=parent_[v];
            if(p<0) continue;
            subtree_[p]+=subtree_[v];
            if(heavy[p]<0 || subtree_[v]>subtree_[heavy[p]]) heavy[p]=v;
        }
        for(int start:roots_) {
            pending.push_back(start);
            while(!pending.empty()) {
                const int chain=pending.back();pending.pop_back();
                for(int v=chain;v!=-1;v=heavy[v]) {
                    head_[v]=chain;index_[v]=static_cast<int>(order_.size());order_.push_back(v);
                    for(auto it=graph[v].rbegin();it!=graph[v].rend();++it)
                        if(parent_[*it]==v && *it!=heavy[v]) pending.push_back(*it);
                }
            }
        }
    }
public:
    struct Interval { int left;int right;bool reversed; };
    HeavyLightDecomposition()=default;
    explicit HeavyLightDecomposition(const Graph& graph,int root=0) { construct(graph,root); }
    HeavyLightDecomposition(const HeavyLightDecomposition&)=default;
    HeavyLightDecomposition(HeavyLightDecomposition&& other) noexcept { swap(other); }
    HeavyLightDecomposition& operator=(HeavyLightDecomposition other) noexcept { swap(other);return *this; }
    void swap(HeavyLightDecomposition& other) noexcept {
        parent_.swap(other.parent_);depth_.swap(other.depth_);component_.swap(other.component_);
        roots_.swap(other.roots_);index_.swap(other.index_);order_.swap(other.order_);
        subtree_.swap(other.subtree_);head_.swap(other.head_);std::swap(root_,other.root_);
    }
    void clear() noexcept { HeavyLightDecomposition{}.swap(*this); }
    void build(const Graph& graph,int root=0) { HeavyLightDecomposition(graph,root).swap(*this); }
    int size() const noexcept { return static_cast<int>(order_.size()); }
    bool empty() const noexcept { return order_.empty(); }
    int root() const noexcept { return root_; }
    int component_count() const noexcept { return static_cast<int>(roots_.size()); }
    int parent(int v) const { check_vertex(v);return parent_[v]; }
    int depth(int v) const { check_vertex(v);return depth_[v]; }
    int component(int v) const { check_vertex(v);return component_[v]; }
    int root_of(int v) const { check_vertex(v);return roots_[component_[v]]; }
    int index(int v) const { check_vertex(v);return index_[v]; }
    int vertex_at(int i) const { check_vertex(i);return order_[i]; }
    const std::vector<int>& order() const noexcept { return order_; }
    bool same_component(int u,int v) const { check_vertex(u);check_vertex(v);return roots_.size()==1 || component_[u]==component_[v]; }
    bool is_ancestor(int u,int v) const {
        check_vertex(u);check_vertex(v);return component_[u]==component_[v] && index_[u]<=index_[v] && index_[v]-index_[u]<subtree_[u];
    }
    std::pair<int,int> subtree_range(int v,bool edges=false) const {
        check_vertex(v);return {index_[v]+static_cast<int>(edges),index_[v]+subtree_[v]};
    }
    int subtree_size(int v) const { check_vertex(v);return subtree_[v]; }
    int lca(int u,int v) const {
        if(!same_component(u,v)) return -1;
        while(head_[u]!=head_[v]) {
            if(depth_[head_[u]]>=depth_[head_[v]]) u=parent_[head_[u]];
            else v=parent_[head_[v]];
        }
        return depth_[u]<depth_[v]?u:v;
    }
    int distance(int u,int v) const {
        const int a=lca(u,v);return a<0?-1:(depth_[u]-depth_[a])+(depth_[v]-depth_[a]);
    }
    int kth_ancestor(int v,std::int64_t k) const {
        check_vertex(v);if(k<0 || k>depth_[v]) return -1;
        while(k>depth_[v]-depth_[head_[v]]) {
            k-=depth_[v]-depth_[head_[v]]+1;v=parent_[head_[v]];
        }
        return order_[index_[v]-static_cast<int>(k)];
    }
    int jump(int u,int v,std::int64_t k) const {
        check_vertex(u);check_vertex(v);
        if(k<0) return -1;
        if(head_[u]==head_[v]) {
            const int delta=index_[v]-index_[u];
            if(k>(delta<0?-delta:delta)) return -1;
            return order_[delta<0?index_[u]-static_cast<int>(k):index_[u]+static_cast<int>(k)];
        }
        if(roots_.size()!=1 && component_[u]!=component_[v]) return -1;
        if(k>static_cast<std::int64_t>(depth_[u])+depth_[v]) return -1;
        if(k==0) return u;
        const int a=lca(u,v);
        const int before=depth_[u]-depth_[a],after=depth_[v]-depth_[a];
        if(k>static_cast<std::int64_t>(before)+after) return -1;
        return k<=before?kth_ancestor(u,k):kth_ancestor(v,static_cast<std::int64_t>(before)+after-k);
    }
    template<class Fn> bool for_each_path(int u,int v,Fn&& fn,bool edges=false) const {
        if(!same_component(u,v)) return false;
        // 軽辺を一つ上がるごとに部分木が倍以上になるため、intの桁数で足りる。
        std::array<Interval,std::numeric_limits<unsigned>::digits> tail;
        std::size_t count=0;
        while(head_[u]!=head_[v]) {
            if(index_[u]>=index_[v]) {
                std::invoke(fn,index_[head_[u]],index_[u]+1,true);u=parent_[head_[u]];
            } else {
                tail[count++]={index_[head_[v]],index_[v]+1,false};v=parent_[head_[v]];
            }
        }
        if(index_[u]>=index_[v]) {
            const int left=index_[v]+static_cast<int>(edges),right=index_[u]+1;
            if(left<right) std::invoke(fn,left,right,true);
        } else {
            const int left=index_[u]+static_cast<int>(edges),right=index_[v]+1;
            if(left<right) std::invoke(fn,left,right,false);
        }
        while(count) { const auto p=tail[--count];std::invoke(fn,p.left,p.right,p.reversed); }
        return true;
    }
    std::vector<Interval> path(int u,int v,bool edges=false) const {
        std::vector<Interval> result;
        for_each_path(u,v,[&](int l,int r,bool rev){result.push_back({l,r,rev});},edges);return result;
    }
    template<class T> std::vector<T> reorder(const std::vector<T>& values) const {
        check_size(values.size());std::vector<T> result;result.reserve(values.size());
        for(int v:order_) result.push_back(values[v]);
        return result;
    }
    template<class T> std::vector<T> restore(const std::vector<T>& values) const {
        check_size(values.size());std::vector<T> result;result.reserve(values.size());
        for(int i:index_) result.push_back(values[i]);
        return result;
    }
    template<class Identity,class Q,class Op> requires /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldProduct<Identity,Q,Op>
    auto path_prod(int u,int v,Identity&& identity,Q&& query,Op&& op,bool edges=false) const {
        if constexpr(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_directional_query<Q>) {
            return path_prod(u,v,std::forward<Identity>(identity),std::forward<Q>(query),std::forward<Op>(op),std::identity{},edges);
        } else {
            const bool connected=same_component(u,v);
#ifndef SKIP_RUNTIME_CHECKS
            if(!connected) throw std::invalid_argument("HLD: disconnected path");
#endif
            using Value=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldQueryValue<Q>;
            Value result(std::forward<Identity>(identity));
            if(!connected) return result;
            // 可換積では通行順の保存を省き、両端から直接結合する。
            while(head_[u]!=head_[v]) {
                if(index_[u]<index_[v]) std::swap(u,v);
                result=std::invoke(op,std::move(result),/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_query(query,index_[head_[u]],index_[u]+1));
                u=parent_[head_[u]];
            }
            if(index_[u]>index_[v]) std::swap(u,v);
            const int left=index_[u]+static_cast<int>(edges),right=index_[v]+1;
            if(left<right) result=std::invoke(op,std::move(result),/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_query(query,left,right));
            return result;
        }
    }
    template<class Identity,class Q,class Op,class Reverse> requires /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldReversedProduct<Identity,Q,Op,Reverse>
    auto path_prod(int u,int v,Identity&& identity,Q&& query,Op&& op,Reverse&& reverse,bool edges=false) const {
        using Value=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldQueryValue<Q>;
        const bool connected=same_component(u,v);
#ifndef SKIP_RUNTIME_CHECKS
        if(!connected) throw std::invalid_argument("HLD: disconnected path");
#endif
        Value empty_result(std::forward<Identity>(identity));
        if(!connected) return empty_result;
        std::optional<Value> from_u,from_v;
        const auto read=[&](int l,int r,bool rev) -> Value {
            Value part=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_query(query,l,r,rev);
            if constexpr(!/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_directional_query<Q>) if(rev) part=std::invoke(reverse,std::move(part));
            return part;
        };
        const auto append=[&](Value part) {
            if(from_u) *from_u=std::invoke(op,std::move(*from_u),std::move(part));
            else from_u.emplace(std::move(part));
        };
        const auto prepend=[&](Value part) {
            if(from_v) *from_v=std::invoke(op,std::move(part),std::move(*from_v));
            else from_v.emplace(std::move(part));
        };
        while(head_[u]!=head_[v]) {
            if(index_[u]>=index_[v]) {
                append(read(index_[head_[u]],index_[u]+1,true));u=parent_[head_[u]];
            } else {
                prepend(read(index_[head_[v]],index_[v]+1,false));v=parent_[head_[v]];
            }
        }
        if(index_[u]>=index_[v]) {
            const int left=index_[v]+static_cast<int>(edges),right=index_[u]+1;
            if(left<right) append(read(left,right,true));
        } else {
            const int left=index_[u]+static_cast<int>(edges),right=index_[v]+1;
            if(left<right) append(read(left,right,false));
        }
        if(from_u && from_v) return Value(std::invoke(op,std::move(*from_u),std::move(*from_v)));
        if(from_u) return std::move(*from_u);
        if(from_v) return std::move(*from_v);
        return empty_result;
    }
    template<class Range> requires (requires(Range& r){r.sum(0,0);} && !requires(Range& r){r.prod(0,0);})
    auto path_prod(int u,int v,Range& range,bool edges=false) const {
        using Value=std::remove_cvref_t<decltype(range.sum(0,0))>;
        return path_prod(u,v,Value{},range,[](const Value& a,const Value& b){return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldArithmetic<Value>::add(a,b);},edges);
    }
    template<template<class,auto,auto> class R,class S,auto Op,auto E>
    S path_prod(int u,int v,const R<S,Op,E>& range,bool edges=false) const {
        return path_prod(u,v,std::invoke(E),range,Op,edges);
    }
    template<template<class,auto,auto> class R,class S,auto Op,auto E,class Reverse>
        requires std::is_invocable_r_v<S,Reverse&,S>
    S path_prod(int u,int v,const R<S,Op,E>& range,Reverse&& reverse,bool edges=false) const {
        return path_prod(u,v,std::invoke(E),range,Op,std::forward<Reverse>(reverse),edges);
    }
    template<template<class,auto,auto,class,auto,auto,auto> class R,class S,auto Op,auto E,class F,auto M,auto C,auto I>
    S path_prod(int u,int v,R<S,Op,E,F,M,C,I>& range,bool edges=false) const {
        return path_prod(u,v,std::invoke(E),range,Op,edges);
    }
    template<template<class,auto,auto,class,auto,auto,auto> class R,class S,auto Op,auto E,class F,auto M,auto C,auto I,class Reverse>
        requires std::is_invocable_r_v<S,Reverse&,S>
    S path_prod(int u,int v,R<S,Op,E,F,M,C,I>& range,Reverse&& reverse,bool edges=false) const {
        return path_prod(u,v,std::invoke(E),range,Op,std::forward<Reverse>(reverse),edges);
    }
    template<class Q> requires /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldQuery<Q>
    auto subtree_prod(int v,Q&& query,bool edges=false) const {
        auto[l,r]=subtree_range(v,edges);return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_query(query,l,r);
    }
    template<class Range,class F> void path_apply(int u,int v,Range& range,const F f,bool edges=false) const {
#ifndef SKIP_RUNTIME_CHECKS
        if(!same_component(u,v)) throw std::invalid_argument("HLD: disconnected path");
#endif
        for_each_path(u,v,[&](int l,int r,bool){range.apply(l,r,f);},edges);
    }
    template<class Range,class F> void subtree_apply(int v,Range& range,const F f,bool edges=false) const {
        auto[l,r]=subtree_range(v,edges);if(l<r) range.apply(l,r,f);
    }
    template<class Range,bool Ordered=false,class T>
    HeavyLightValues<Range,Ordered> with_values(const std::vector<T>& values) const &;
    template<class Range,bool Ordered=false,class T>
    HeavyLightValues<Range,Ordered> with_values(const std::vector<T>& values) &&;
};
#endif

#ifndef SIRKIT_HLD_OWNED_RANGE_TRAITS_DEFINED
#define SIRKIT_HLD_OWNED_RANGE_TRAITS_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class Range> struct HldOwnedRangeTraits {
    using Value=HldQueryValue<Range>;
    static Value identity() {return Value{};}
    static Value combine(const Value& a,const Value& b) {return HldArithmetic<Value>::add(a,b);}
};
template<template<class,auto,auto> class R,class S,auto Op,auto E>
struct HldOwnedRangeTraits<R<S,Op,E>> {
    using Value=S;
    static Value identity(){return std::invoke(E);}
    static Value combine(const Value& a,const Value& b){return std::invoke(Op,a,b);}
};
template<template<class,auto,auto,class,auto,auto,auto> class R,class S,auto Op,auto E,class F,auto M,auto C,auto I>
struct HldOwnedRangeTraits<R<S,Op,E,F,M,C,I>> {
    using Value=S;
    static Value identity(){return std::invoke(E);}
    static Value combine(const Value& a,const Value& b){return std::invoke(Op,a,b);}
};
}
#endif

#ifndef SIRKIT_HLD_SEGMENT_ACCESS_DEFINED
#define SIRKIT_HLD_SEGMENT_ACCESS_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class Range> struct HldSegmentAccess {
    static constexpr bool enabled=[] {
        if constexpr(requires {typename Range::HldSegmentTag;}) {
            using T=typename HldOwnedRangeTraits<Range>::Value;
            return sizeof(T)==16 && std::is_trivially_copyable_v<T>;
        } else return false;
    }();
};
}
#endif

#ifndef SIRKIT_HEAVY_LIGHT_VALUES_DEFINED
#define SIRKIT_HEAVY_LIGHT_VALUES_DEFINED
template<class Range,bool Ordered> class HeavyLightValues {
    using Traits=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldOwnedRangeTraits<Range>;
public:
    using value_type=typename Traits::Value;
private:
    struct State {
        HeavyLightDecomposition layout;
        Range forward;
        struct NoReverse {};
        [[no_unique_address]] std::conditional_t<Ordered,std::optional<Range>,NoReverse> backward;
        static Range make_range(const std::vector<value_type>& values) {
            if constexpr(std::constructible_from<Range,const std::vector<value_type>&>) return Range(values);
            else {
                if(values.size()>static_cast<std::size_t>(std::numeric_limits<int>::max())) throw std::length_error("HLD: range size");
                Range range(static_cast<int>(values.size()));
                for(int i=0;i<static_cast<int>(values.size());++i) range.add(i,values[i]);
                return range;
            }
        }
        template<class Layout> State(Layout&& source,const std::vector<value_type>& ordered)
            :layout(std::forward<Layout>(source)),forward(make_range(ordered)) {
            if constexpr(Ordered) {
                auto reversed=ordered;std::reverse(reversed.begin(),reversed.end());backward.emplace(make_range(reversed));
            }
        }
    };
    std::unique_ptr<State> state_;
    static const HeavyLightDecomposition& empty_layout() {static const HeavyLightDecomposition h;return h;}
    const HeavyLightDecomposition& layout() const noexcept {return state_?state_->layout:empty_layout();}
    static value_type range_get(Range& range,int p) {
        if constexpr(requires {range.get(p);}) return range.get(p);
        else return range.sum(p,p+1);
    }
    static void range_set(Range& range,int p,const value_type& x) {
        if constexpr(requires {range.set(p,x);}) range.set(p,x);
        else range.add(p,/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldArithmetic<value_type>::subtract(x,range_get(range,p)));
    }
    void check_vertex(int v) const {(void)layout().index(v);}
    static value_type range_query(Range& range,int l,int r) {
        // 所有BITの一点区間は、二つの累積和を求めず直接読む。
        if constexpr(requires {range.sum(l,r);range.get(l);} && !requires {range.prod(l,r);})
            if(r-l==1) return range.get(l);
        return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_query(range,l,r);
    }
    value_type query(int l,int r,bool reverse) {
        if constexpr(Ordered) if(reverse) return range_query(*state_->backward,size()-r,size()-l);
        return range_query(state_->forward,l,r);
    }
public:
    HeavyLightValues()=default;
    HeavyLightValues(const Graph& graph,const std::vector<value_type>& values,int root=0)
        :HeavyLightValues(HeavyLightDecomposition(graph,root),values) {}
    HeavyLightValues(const HeavyLightDecomposition& source,const std::vector<value_type>& values) {
        auto ordered=source.reorder(values);state_=std::make_unique<State>(source,ordered);
    }
    HeavyLightValues(HeavyLightDecomposition&& source,const std::vector<value_type>& values) {
        // valuesがsource.order()を参照する場合も、移動前に並べ替える。
        auto ordered=source.reorder(values);state_=std::make_unique<State>(std::move(source),ordered);
    }
    HeavyLightValues(const HeavyLightValues& other) {
        if(other.state_) state_=std::make_unique<State>(*other.state_);
    }
    HeavyLightValues(HeavyLightValues&&)=default;
    HeavyLightValues& operator=(HeavyLightValues other) noexcept {swap(other);return *this;}
    void swap(HeavyLightValues& other) noexcept {state_.swap(other.state_);}
    void clear() noexcept {state_.reset();}
    void build(const Graph& graph,const std::vector<value_type>& values,int root=0) {HeavyLightValues(graph,values,root).swap(*this);}
    int size() const noexcept {return layout().size();}
    bool empty() const noexcept {return layout().empty();}
    int root() const noexcept {return layout().root();}
    int component_count() const noexcept {return layout().component_count();}
    int parent(int v) const {return layout().parent(v);}
    int depth(int v) const {return layout().depth(v);}
    int component(int v) const {return layout().component(v);}
    int root_of(int v) const {return layout().root_of(v);}
    int index(int v) const {return layout().index(v);}
    int vertex_at(int i) const {return layout().vertex_at(i);}
    const std::vector<int>& order() const noexcept {return layout().order();}
    bool same_component(int u,int v) const {return layout().same_component(u,v);}
    bool is_ancestor(int u,int v) const {return layout().is_ancestor(u,v);}
    std::pair<int,int> subtree_range(int v,bool edges=false) const {return layout().subtree_range(v,edges);}
    int subtree_size(int v) const {return layout().subtree_size(v);}
    int lca(int u,int v) const {return layout().lca(u,v);}
    int distance(int u,int v) const {return layout().distance(u,v);}
    int kth_ancestor(int v,std::int64_t k) const {return layout().kth_ancestor(v,k);}
    int jump(int u,int v,std::int64_t k) const {return layout().jump(u,v,k);}
    template<class Fn> bool for_each_path(int u,int v,Fn&& fn,bool edges=false) const {return layout().for_each_path(u,v,std::forward<Fn>(fn),edges);}
    std::vector<HeavyLightDecomposition::Interval> path(int u,int v,bool edges=false) const {return layout().path(u,v,edges);}
    template<class T> std::vector<T> reorder(const std::vector<T>& values) const {return layout().reorder(values);}
    template<class T> std::vector<T> restore(const std::vector<T>& values) const {return layout().restore(values);}
    value_type get(int v) requires (requires(Range& r){r.get(0);} || requires(Range& r){r.sum(0,0);}) {
        const int p=index(v);return range_get(state_->forward,p);
    }
    void set(int v,const value_type x) requires (requires(Range& r){r.set(0,x);} || requires(Range& r){r.add(0,x-x);}) {
        const int p=index(v);range_set(state_->forward,p,x);
        if constexpr(Ordered) range_set(*state_->backward,size()-1-p,x);
    }
    void add(int v,const value_type x) requires requires(Range& r){r.add(0,x);} {
        const int p=index(v);state_->forward.add(p,x);
        if constexpr(Ordered) state_->backward->add(size()-1-p,x);
    }
    template<class F> void apply(int v,const F f) requires requires(Range& r){r.apply(0,f);} {
        const int p=index(v);state_->forward.apply(p,f);
        if constexpr(Ordered) state_->backward->apply(size()-1-p,f);
    }
    value_type path_prod(int u,int v,bool edges=false) {
        if constexpr(Ordered) {
            auto interval=[&](int l,int r,bool reverse){return query(l,r,reverse);};
            return layout().path_prod(u,v,Traits::identity(),interval,Traits::combine,edges);
        } else {
            auto interval=[&](int l,int r){return query(l,r,false);};
            return layout().path_prod(u,v,Traits::identity(),interval,Traits::combine,edges);
        }
    }
    value_type subtree_prod(int v,bool edges=false) {
        auto[l,r]=subtree_range(v,edges);return query(l,r,false);
    }
    template<class F> void path_apply(int u,int v,const F f,bool edges=false) requires requires(Range& r){r.apply(0,0,f);} {
#ifndef SKIP_RUNTIME_CHECKS
        if(!same_component(u,v)) throw std::invalid_argument("HLD: disconnected path");
#endif
        for_each_path(u,v,[&](int l,int r,bool) {
            state_->forward.apply(l,r,f);
            if constexpr(Ordered) state_->backward->apply(size()-r,size()-l,f);
        },edges);
    }
    template<class F> void subtree_apply(int v,const F f,bool edges=false) requires requires(Range& r){r.apply(0,0,f);} {
        auto[l,r]=subtree_range(v,edges);if(l==r) return;
        state_->forward.apply(l,r,f);
        if constexpr(Ordered) state_->backward->apply(size()-r,size()-l,f);
    }
    std::vector<value_type> values() {
        if(!state_) return {};
        if constexpr(requires {state_->forward.values();}) return restore(state_->forward.values());
        else {
            std::vector<value_type> ordered;ordered.reserve(static_cast<std::size_t>(size()));
            for(int i=0;i<size();++i) ordered.push_back(range_get(state_->forward,i));
            return restore(ordered);
        }
    }
};
template<class Range,bool Ordered,class T>
HeavyLightValues<Range,Ordered> HeavyLightDecomposition::with_values(const std::vector<T>& values) const & {return HeavyLightValues<Range,Ordered>(*this,values);}
template<class Range,bool Ordered,class T>
HeavyLightValues<Range,Ordered> HeavyLightDecomposition::with_values(const std::vector<T>& values) && {return HeavyLightValues<Range,Ordered>(std::move(*this),values);}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
