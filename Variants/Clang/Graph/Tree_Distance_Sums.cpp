// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Tree_Distance_Sums.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_distance_sums.md
//
// API一覧
// TreeDistanceSums<T>(n,edges,weights={},root=0): 0始まりの重み付き木。空のweightsは全頂点を0で初期化。
//   TreeDistanceSumEdge<T>{from,to,length=1}。d(u,v)は木のパスにある辺長の和。構築O(n)時間・領域。
// add(v,delta): 頂点重みW[v]にdeltaを加える。sum_at(v): Σ W[r]*d(r,v) を返す。
// sum_path(u,v): u..vの両端を含むすべての頂点xについてsum_at(x)の総和を返す。
//   更新と照会はO(log²(n+1))時間。Tは加減乗算と整数からの変換を持つ環の値。
//   組み込み整数Tは型幅で周回。区間照会は遅延伝播を行う。
//   LCAまでの辺長を、根からxまでの辺長と各子部分木重みの積の和で集約する。
//   根からの経路に集約を重ねると、深さdの辺はdepth(x)-d+1回現れる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
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

#ifndef SIRKIT_HLD_QUERY_HELPERS_DEFINED
#define SIRKIT_HLD_QUERY_HELPERS_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline int HldVertexCount(std::size_t vertices) {
    if (vertices > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("HLD: int size overflow");
    return static_cast<int>(vertices);
}

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
        const int n=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldVertexCount(graph.size());
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
            std::invoke(fn,left,right,false);
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
            using Value=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldQueryValue<Q>;
            if(!same_component(u,v)) {
#ifndef SKIP_RUNTIME_CHECKS
                throw std::invalid_argument("HLD: disconnected path");
#else
                return Value(std::forward<Identity>(identity));
#endif
            }
            Value result(std::forward<Identity>(identity));
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
        if(!same_component(u,v)) {
#ifndef SKIP_RUNTIME_CHECKS
            throw std::invalid_argument("HLD: disconnected path");
#else
            return Value(std::forward<Identity>(identity));
#endif
        }
        Value empty_result(std::forward<Identity>(identity));
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
            append(read(left,right,false));
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

#ifndef SIRKIT_SEGMENT_ARRAY_CAPACITY_DEFINED
#define SIRKIT_SEGMENT_ARRAY_CAPACITY_DEFINED
namespace detail {
inline std::size_t CheckedSegmentLeafBase(std::size_t count, std::size_t limit, const char* error) {
    if (count == 0) return 1;
    if (count > std::bit_floor(limit)) throw std::length_error(error);
    return std::bit_ceil(count);
}

inline int CheckedSegmentArrayCount(std::size_t count, const char* error) {
    if (count > static_cast<std::size_t>(std::numeric_limits<int>::max())) throw std::length_error(error);
    return static_cast<int>(count);
}
}
#endif

#ifndef SIRKIT_LAZY_SEGMENT_TREE_DEFINED
#define SIRKIT_LAZY_SEGMENT_TREE_DEFINED
template<class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
class lazy_segtree {
    static_assert(std::is_invocable_r_v<S, decltype(op), const S&, const S&>);
    static_assert(std::is_invocable_r_v<S, decltype(e)>);
    static_assert(std::is_invocable_r_v<S, decltype(mapping), const F&, const S&>
        || std::is_invocable_r_v<S, decltype(mapping), const F&, const S&, std::size_t>);
    static_assert(std::is_invocable_r_v<F, decltype(composition), const F&, const F&>);
    static_assert(std::is_invocable_r_v<F, decltype(id)>);
    S value_identity_;
    F update_identity_;
    std::size_t n_ = 0, base_ = 1, height_ = 0;
    std::vector<S> data_;
    std::vector<F> lazy_;
    std::vector<std::uint8_t> pending_;
    static std::size_t leaf_base(std::size_t n) {
        const auto limit = std::min({std::vector<S>{}.max_size() / 2,
                                     std::vector<F>{}.max_size(), std::vector<std::uint8_t>{}.max_size()});
        return detail::CheckedSegmentLeafBase(n, limit, "lazy_segtree: size");
    }
    static std::size_t checked_size(int n) {
#ifndef SKIP_RUNTIME_CHECKS
        if (n < 0) throw std::invalid_argument("lazy_segtree: size");
#endif
        return static_cast<std::size_t>(n);
    }
    static int vector_size(const std::vector<S>& values) {
        return detail::CheckedSegmentArrayCount(values.size(), "lazy_segtree: size");
    }
    void check_index(int i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (i < 0 || static_cast<std::size_t>(i) >= n_) throw std::out_of_range("lazy_segtree: index");
#else
        (void)i;
#endif
    }
    void check_range(int l, int r) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (l < 0 || l > r || static_cast<std::size_t>(r) > n_) throw std::out_of_range("lazy_segtree: range");
#else
        (void)l; (void)r;
#endif
    }
    static std::size_t aligned_node(std::size_t value) {
        if (value == 0) std::unreachable();
        return value >> std::countr_zero(value);
    }
    void pull(std::size_t p) { data_[p] = std::invoke(op, data_[2 * p], data_[2 * p + 1]); }
    void act(std::size_t p, const F& f, std::size_t length) {
        if constexpr (std::is_invocable_r_v<S, decltype(mapping), const F&, const S&>)
            data_[p] = std::invoke(mapping, f, data_[p]);
        else data_[p] = std::invoke(mapping, f, data_[p], length);
        if (p < base_) {
            if (pending_[p]) lazy_[p] = std::invoke(composition, f, lazy_[p]);
            else { lazy_[p] = f; pending_[p] = 1; }
        }
    }
    void push(std::size_t p, std::size_t length) {
        if (!pending_[p]) return;
        act(2 * p, lazy_[p], length / 2);
        act(2 * p + 1, lazy_[p], length / 2);
        pending_[p] = 0;
    }
    void push_path(std::size_t leaf) {
        for (auto level = height_; level; --level) push(leaf >> level, std::size_t{1} << level);
    }
    void push_boundary(std::size_t l, std::size_t r) {
        for (auto level = height_; level; --level) {
            const auto length = std::size_t{1} << level;
            if (l & (length - 1)) push(l >> level, length);
            if (r & (length - 1)) push((r - 1) >> level, length);
        }
    }
    void pull_path(std::size_t leaf) {
        for (leaf >>= 1; leaf; leaf >>= 1) pull(leaf);
    }
public:
    lazy_segtree() : lazy_segtree(0) {}
    explicit lazy_segtree(int n)
        : value_identity_(std::invoke(e)), update_identity_(std::invoke(id)), n_(checked_size(n)), base_(leaf_base(n_)),
          data_(n_ ? 2 * base_ : 0, value_identity_), lazy_(n_ ? base_ : 0, update_identity_), pending_(n_ ? base_ : 0, 0) {
        for (auto p = base_; p > 1; p >>= 1) ++height_;
    }
    explicit lazy_segtree(const std::vector<S>& values) : lazy_segtree(vector_size(values)) {
        if (!n_) return;
        std::copy(values.begin(), values.end(), data_.begin() + base_);
        for (auto p = base_; p > 1;) pull(--p);
    }
    lazy_segtree(const lazy_segtree&) = default;
    lazy_segtree& operator=(const lazy_segtree& other) {
        if (this != &other) { lazy_segtree replacement(other); swap(replacement); }
        return *this;
    }
    lazy_segtree(lazy_segtree&& other)
        : value_identity_(other.value_identity_), update_identity_(other.update_identity_),
          n_(other.n_), base_(other.base_), height_(other.height_), data_(std::move(other.data_)),
          lazy_(std::move(other.lazy_)), pending_(std::move(other.pending_)) {
        other.clear();
    }
    lazy_segtree& operator=(lazy_segtree&& other) {
        if (this != &other) { lazy_segtree replacement(std::move(other)); swap(replacement); }
        return *this;
    }
    int size() const noexcept { return static_cast<int>(n_); }
    bool empty() const noexcept { return n_ == 0; }
    S get(int i) {
        check_index(i);
        const auto leaf = base_ + i;
        push_path(leaf);
        return data_[leaf];
    }
    void set(int i, S value) {
        check_index(i);
        const auto leaf = base_ + i;
        push_path(leaf); data_[leaf] = value; pull_path(leaf);
    }
    void apply(int i, F f) {
        check_index(i);
        const auto leaf = base_ + i;
        push_path(leaf); act(leaf, f, 1); pull_path(leaf);
    }
    void apply(int l, int r, F f) {
        check_range(l, r);
        if (l == r) return;
        const auto begin = base_ + static_cast<std::size_t>(l);
        const auto end = base_ + static_cast<std::size_t>(r);
        push_boundary(begin, end);
        auto left = begin, right = end;
        std::size_t length = 1;
        while (left < right) {
            if (left & 1) act(left++, f, length);
            if (right & 1) act(--right, f, length);
            left >>= 1; right >>= 1; length *= 2;
        }
        for (std::size_t level = 1; level <= height_; ++level) {
            const auto span = std::size_t{1} << level;
            if (begin & (span - 1)) pull(begin >> level);
            if (end & (span - 1)) pull((end - 1) >> level);
        }
    }
    S prod(int l, int r) {
        check_range(l, r);
        if (l == r) return value_identity_;
        auto begin = base_ + static_cast<std::size_t>(l);
        auto end = base_ + static_cast<std::size_t>(r);
        push_boundary(begin, end);
        // コピー専用型に、局所値を返す際の暗黙ムーブを要求しない。
        if constexpr (!std::is_move_constructible_v<S>) {
            S left = value_identity_, right = value_identity_;
            for (; begin < end; begin >>= 1, end >>= 1) {
                if (begin & 1) left = std::invoke(op, left, data_[begin++]);
                if (end & 1) right = std::invoke(op, data_[--end], right);
            }
            return std::invoke(op, left, right);
        } else {
            auto step = begin & (~begin + 1);
            if (step > end - begin) {
                S result = data_[aligned_node(end) - 1];
                end -= end & (~end + 1);
                while (begin < end) {
                    result = std::invoke(op, data_[aligned_node(end) - 1], result);
                    end -= end & (~end + 1);
                }
                return result;
            }
            S left = data_[aligned_node(begin)];
            begin += step;
            while (begin < end && (step = begin & (~begin + 1)) <= end - begin) {
                left = std::invoke(op, left, data_[aligned_node(begin)]);
                begin += step;
            }
            if (begin == end) return left;
            S right = data_[aligned_node(end) - 1];
            end -= end & (~end + 1);
            while (begin < end) {
                right = std::invoke(op, data_[aligned_node(end) - 1], right);
                end -= end & (~end + 1);
            }
            return std::invoke(op, left, right);
        }
    }

    S all_prod() { return n_ ? data_[1] : value_identity_; }
    template<bool (*pred)(S)> int max_right(int l) {
        return max_right(l, pred);
    }
    template<class Pred> int max_right(int l, Pred pred) {
        check_range(l, l);
#ifndef SKIP_RUNTIME_CHECKS
        if (!pred(value_identity_)) throw std::invalid_argument("lazy_segtree: predicate(identity)");
#endif
        if (static_cast<std::size_t>(l) == n_) return size();
        auto node = base_ + l;
        push_path(node);
        std::size_t length = 1;
        S accumulated = value_identity_;
        while (true) {
            while (!(node & 1)) { node >>= 1; length *= 2; }
            S candidate = std::invoke(op, accumulated, data_[node]);
            if (!pred(candidate)) {
                while (node < base_) {
                    push(node, length); node *= 2; length /= 2;
                    S next = std::invoke(op, accumulated, data_[node]);
                    if (pred(next)) { accumulated = std::move(next); ++node; }
                }
                return static_cast<int>(std::min(n_, node - base_));
            }
            accumulated = std::move(candidate);
            ++node;
            if ((node & (node - 1)) == 0) return size();
        }
    }
    template<bool (*pred)(S)> int min_left(int r) {
        return min_left(r, pred);
    }
    template<class Pred> int min_left(int r, Pred pred) {
        check_range(r, r);
#ifndef SKIP_RUNTIME_CHECKS
        if (!pred(value_identity_)) throw std::invalid_argument("lazy_segtree: predicate(identity)");
#endif
        if (r == 0) return 0;
        auto node = base_ + r;
        push_path(node - 1);
        std::size_t length = 1;
        S accumulated = value_identity_;
        while (true) {
            --node;
            while (node > 1 && (node & 1)) { node >>= 1; length *= 2; }
            S candidate = std::invoke(op, data_[node], accumulated);
            if (!pred(candidate)) {
                while (node < base_) {
                    push(node, length); node = 2 * node + 1; length /= 2;
                    S next = std::invoke(op, data_[node], accumulated);
                    if (pred(next)) { accumulated = std::move(next); --node; }
                }
                return static_cast<int>(node + 1 - base_);
            }
            accumulated = std::move(candidate);
            if ((node & (node - 1)) == 0) return 0;
        }
    }
    std::vector<S> values() {
        if (!n_) return {};
        for (std::size_t begin = 1, length = base_; begin < base_; begin *= 2, length /= 2)
            for (auto p = begin; p < 2 * begin; ++p) push(p, length);
        return std::vector<S>(data_.begin() + base_, data_.begin() + base_ + n_);
    }
    void assign(const std::vector<S>& values) {
        lazy_segtree replacement(values); swap(replacement);
    }
    void reset(int n) {
        lazy_segtree replacement(n); swap(replacement);
    }
    void reset() {
        std::fill(data_.begin(), data_.end(), value_identity_);
        std::fill(lazy_.begin(), lazy_.end(), update_identity_);
        std::fill(pending_.begin(), pending_.end(), 0);
    }
    void clear() noexcept {
        data_.clear(); lazy_.clear(); pending_.clear(); n_ = 0; base_ = 1; height_ = 0;
    }
    void swap(lazy_segtree& other) noexcept(std::is_nothrow_swappable_v<S> && std::is_nothrow_swappable_v<F>) {
        using std::swap;
        swap(value_identity_, other.value_identity_); swap(update_identity_, other.update_identity_);
        swap(n_, other.n_); swap(base_, other.base_); swap(height_, other.height_);
        data_.swap(other.data_); lazy_.swap(other.lazy_); pending_.swap(other.pending_);
    }
};
#endif

#ifndef SIRKIT_TREE_DISTANCE_SUMS_DEFINED
#define SIRKIT_TREE_DISTANCE_SUMS_DEFINED

template <typename T>
struct TreeDistanceSumEdge
{
    int from;
    int to;
    T length = T{1};
};

template <typename T = std::uint64_t>
class TreeDistanceSums
{
    using Number = std::conditional_t<std::is_integral_v<T>, std::uint64_t, T>;
    static_assert(!std::is_integral_v<T> || (sizeof(T) <= sizeof(std::uint64_t) && !std::is_same_v<T, bool>));

    struct Moments
    {
        Number weighted_length{};
        Number weighted_depth{};
        Number length{};
        Number depth_length{};
    };

    static Moments combine(const Moments &a, const Moments &b)
    {
        return {a.weighted_length + b.weighted_length, a.weighted_depth + b.weighted_depth,
                a.length + b.length, a.depth_length + b.depth_length};
    }

    static Moments identity()
    {
        return {};
    }

    static Moments update(const Number &delta, Moments value)
    {
        value.weighted_length = value.weighted_length + delta * value.length;
        value.weighted_depth = value.weighted_depth + delta * value.depth_length;
        return value;
    }

    static Number compose(const Number &a, const Number &b)
    {
        return a + b;
    }

    static Number zero()
    {
        return {};
    }

    using Aggregates = lazy_segtree<Moments, combine, identity, Number, update, compose, zero>;

public:
    TreeDistanceSums(int n, std::span<const TreeDistanceSumEdge<T>> edges,
                     std::span<const T> weights = {}, int root = 0)
    {
        if (n < 0)
        {
            throw std::length_error("TreeDistanceSums: negative vertex count");
        }
        if (edges.size() != static_cast<std::size_t>(n == 0 ? 0 : n - 1) ||
            (!weights.empty() && weights.size() != static_cast<std::size_t>(n)))
        {
            throw std::invalid_argument("TreeDistanceSums: inconsistent sizes");
        }
        Graph graph(n);
        for (const auto &[u, v, length] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (static_cast<std::size_t>(u) >= graph.size() || static_cast<std::size_t>(v) >= graph.size())
            {
                throw std::out_of_range("TreeDistanceSums: invalid vertex");
            }
#endif
            graph[u].push_back(v);
            graph[v].push_back(u);
        }
        layout_.build(graph, root);
        std::vector<Number> edge_length(n), subtree_weight(n);
        for (const auto &[u, v, length] : edges)
        {
            const int child = layout_.parent(u) == v ? u : v;
            edge_length[child] = static_cast<Number>(length);
        }
        depth_length_.resize(n);
        prefix_length_.resize(n);
        for (const int v : layout_.order())
        {
            const int parent = layout_.parent(v);
            if (parent != -1)
            {
                depth_length_[v] = depth_length_[parent] + edge_length[v];
                prefix_length_[v] = prefix_length_[parent] + depth_length_[v];
            }
            if (!weights.empty())
            {
                subtree_weight[v] = static_cast<Number>(weights[v]);
                total_weight_ = total_weight_ + subtree_weight[v];
                weighted_origin_ = weighted_origin_ + subtree_weight[v] * depth_length_[v];
            }
        }
        std::vector<Moments> leaves(n);
        for (const int v : std::views::reverse(layout_.order()))
        {
            const int parent = layout_.parent(v);
            if (parent != -1)
            {
                subtree_weight[parent] = subtree_weight[parent] + subtree_weight[v];
            }
            const auto length = edge_length[v];
            const auto depth = static_cast<Number>(layout_.depth(v));
            const auto weighted = length * subtree_weight[v];
            leaves[layout_.index(v)] = {weighted, weighted * depth, length, length * depth};
        }
        aggregates_.assign(leaves);
    }

    void add(int v, const T &delta)
    {
        const auto amount = static_cast<Number>(delta);
        layout_.path_apply(layout_.root(), v, aggregates_, amount, true);
        total_weight_ = total_weight_ + amount;
        weighted_origin_ = weighted_origin_ + amount * depth_length_[v];
    }

    [[nodiscard]] T sum_at(int v)
    {
        const auto moment = origin_moments(v);
        return static_cast<T>(vertex_value(v, moment));
    }

    [[nodiscard]] T sum_path(int u, int v)
    {
        const int middle = layout_.lca(u, v);
        const auto middle_moments = origin_moments(middle);
        const auto middle_prefix = prefix_value(middle, middle_moments);
        const auto first = u == middle ? middle_prefix : prefix_value(u, origin_moments(u));
        const auto last = v == middle ? middle_prefix : prefix_value(v, origin_moments(v));
        return static_cast<T>(first + last - middle_prefix - middle_prefix + vertex_value(middle, middle_moments));
    }

private:
    [[nodiscard]] Moments origin_moments(int v)
    {
        return layout_.path_prod(layout_.root(), v, Moments{}, aggregates_, combine, true);
    }

    [[nodiscard]] Number vertex_value(int v, const Moments &value) const
    {
        return total_weight_ * depth_length_[v] + weighted_origin_ - value.weighted_length - value.weighted_length;
    }

    [[nodiscard]] Number prefix_value(int v, const Moments &value) const
    {
        const auto count = static_cast<Number>(static_cast<std::uint64_t>(layout_.depth(v)) + 1);
        const auto lca_sum = count * value.weighted_length - value.weighted_depth;
        return total_weight_ * prefix_length_[v] + weighted_origin_ * count - lca_sum - lca_sum;
    }

    HeavyLightDecomposition layout_;
    Aggregates aggregates_;
    std::vector<Number> depth_length_;
    std::vector<Number> prefix_length_;
    Number total_weight_{};
    Number weighted_origin_{};
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
