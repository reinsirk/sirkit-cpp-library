// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
// ACL形式の署名・残余状態・コピーと、追加APIの互換性を確認する。
#ifndef MAX_FLOW_SOURCE
#define MAX_FLOW_SOURCE "../../../Graph/Max_Flow.cpp"
#endif
#include MAX_FLOW_SOURCE
#include MAX_FLOW_SOURCE
#include <algorithm>
#include <cstdint>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <source_location>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
namespace {
using I=std::int64_t;
using G=mf_graph<I>;
using E=G::edge;
static_assert(std::is_default_constructible_v<G>);
static_assert(std::is_constructible_v<G,int>);
static_assert(std::is_same_v<decltype(&G::add_edge),int(G::*)(int,int,I)>);
static_assert(std::is_same_v<decltype(static_cast<E(G::*)(int)>(&G::get_edge)),E(G::*)(int)>);
static_assert(std::is_same_v<decltype(static_cast<std::vector<E>(G::*)()>(&G::edges)),std::vector<E>(G::*)()>);
static_assert(std::is_same_v<decltype(&G::change_edge),void(G::*)(int,I,I)>);
static_assert(std::is_same_v<decltype(static_cast<I(G::*)(int,int)>(&G::flow)),I(G::*)(int,int)>);
static_assert(std::is_same_v<decltype(static_cast<I(G::*)(int,int,I)>(&G::flow)),I(G::*)(int,int,I)>);
static_assert(std::is_same_v<decltype(static_cast<std::vector<bool>(G::*)(int)>(&G::min_cut)),std::vector<bool>(G::*)(int)>);
static_assert(std::is_same_v<decltype(static_cast<E(G::*)(int)const>(&G::get_edge)),E(G::*)(int)const>);
static_assert(std::is_same_v<decltype(static_cast<std::vector<E>(G::*)()const>(&G::edges)),std::vector<E>(G::*)()const>);
static_assert(std::is_same_v<decltype(static_cast<std::vector<bool>(G::*)(int)const>(&G::min_cut)),std::vector<bool>(G::*)(int)const>);
static_assert(std::is_aggregate_v<E> && std::is_standard_layout_v<E>);
static_assert(std::is_same_v<decltype(E::from),int> && std::is_same_v<decltype(E::to),int>);
static_assert(std::is_same_v<decltype(E::cap),I> && std::is_same_v<decltype(E::flow),I>);
std::uint64_t checks=0;
void check(bool ok,const std::source_location where=std::source_location::current()) {
    ++checks;
    if(!ok){std::fprintf(stderr,"mf_graph API failed %s:%u\n",where.file_name(),where.line());std::abort();}
}
template<class Exception,class Fn> void expect(Fn&& fn) {
    bool caught=false;
    try{fn();}catch(const Exception&){caught=true;}
    check(caught);
}
bool same(const std::vector<E>& a,const std::vector<E>& b) {
    if(a.size()!=b.size())return false;
    for(std::size_t i=0;i<a.size();++i)
        if(a[i].from!=b[i].from||a[i].to!=b[i].to||a[i].cap!=b[i].cap||a[i].flow!=b[i].flow)return false;
    return true;
}
I cut_reference(int n,const std::vector<E>& edges,int s,int t) {
    I answer=std::numeric_limits<I>::max();
    for(unsigned mask=0;mask<(1U<<n);++mask) {
        if(!(mask>>s&1U)||(mask>>t&1U))continue;
        I capacity=0;
        for(const auto& e:edges)if((mask>>e.from&1U)&&!(mask>>e.to&1U))capacity+=e.cap;
        answer=std::min(answer,capacity);
    }
    return answer;
}
void state(G& g,int s,int t,I total) {
    const auto edges=g.edges();std::vector<I> balance(static_cast<std::size_t>(g.size()));
    for(std::size_t i=0;i<edges.size();++i) {
        const E e=g.get_edge(static_cast<int>(i));
        check(e.from==edges[i].from&&e.to==edges[i].to&&e.cap==edges[i].cap&&e.flow==edges[i].flow);
        check(e.cap>=0&&e.flow>=0&&e.flow<=e.cap);
        balance[e.from]-=e.flow;balance[e.to]+=e.flow;
    }
    for(int v=0;v<g.size();++v)check(balance[v]==(v==s?-total:v==t?total:0));
}
void primary() {
    G g;check(g.size()==0&&g.edge_count()==0&&g.edges().empty());
    expect<std::length_error>([&]{g.reserve_edges(static_cast<std::size_t>(std::numeric_limits<int>::max())+1);});
    g.reserve_edges(16);g.reset(4);
    const std::vector<E> input{{0,0,7,0},{0,1,5,0},{0,1,3,0},{1,0,4,0},
                               {1,2,5,0},{2,3,8,0},{0,2,4,0},{1,3,2,0}};
    for(std::size_t i=0;i<input.size();++i) {
        const auto e=input[i];check(g.add_edge(e.from,e.to,e.cap)==static_cast<int>(i));
    }
    check(g.size()==4&&g.edge_count()==input.size()&&same(g.edges(),input));
    auto independent=g;const I expected=cut_reference(4,input,0,3);check(expected==10);
    check(g.flow(0,3,I{0})==0);state(g,0,3,0);
    check(g.flow(0,3,I{3})==3);state(g,0,3,3);
    check(g.flow(0,3,I{2})==2);state(g,0,3,5);
    check(g.flow(0,3)==expected-5);state(g,0,3,expected);
    check(g.flow(0,3)==0);check(g.get_edge(0).flow==0);
    const G& readonly=g;
    check(same(readonly.edges(),g.edges())&&readonly.get_edge(0).cap==7);
    g.change_edge(0,7,6);check(readonly.get_edge(0).flow==6);
    g.change_edge(0,7,0);
    const auto cut=g.min_cut(0);check(cut.size()==4&&cut[0]&&!cut[3]);
    check(readonly.min_cut(0)==cut);
    I capacity=0;for(const auto e:g.edges())if(cut[e.from]&&!cut[e.to])capacity+=e.cap;
    check(capacity==expected);
    check(same(independent.edges(),input));check(independent.flow(0,3)==expected);
    const auto independent_saturated=independent.edges();
    for(std::size_t i=0;i<input.size();++i)g.change_edge(static_cast<int>(i),input[i].cap,0);
    check(same(g.edges(),input));g.change_edge(1,1,0);auto changed=g.edges();
    check(changed[1].cap==1&&changed[2].cap==3&&changed[3].cap==4);
    const I next_expected=cut_reference(4,changed,0,3);check(g.flow(0,3)==next_expected);state(g,0,3,next_expected);
    auto copied=g;copied.reset(2);copied.add_edge(0,1,19);check(copied.flow(0,1)==19);
    check(g.size()==4&&g.get_edge(1).cap==1&&same(independent.edges(),independent_saturated));
    auto moved=std::move(g);check(g.size()==0&&g.edge_count()==0&&g.edges().empty());
    check(moved.size()==4&&moved.flow(0,3)==0);g=moved;
    check(g.size()==4&&same(g.edges(),moved.edges()));g.reset();check(g.size()==0&&moved.size()==4);
    g=std::move(copied);check(copied.size()==0&&g.size()==2&&g.get_edge(0).flow==19);
    moved.swap(g);check(moved.size()==2&&g.size()==4);check(moved.get_edge(0).cap==19);
    // ACLで制約外の同じ始点・終点は、従来からの追加仕様として0。
    const auto before=g.edges();check(g.flow(1,1)==0);check(same(before,g.edges()));
#ifndef SKIP_RUNTIME_CHECKS
    const auto unchanged=g.edges();
    expect<std::out_of_range>([&]{(void)g.get_edge(-1);});
    expect<std::out_of_range>([&]{g.change_edge(-1,0,0);});
    expect<std::out_of_range>([&]{(void)g.get_edge(static_cast<int>(g.edge_count()));});
    expect<std::invalid_argument>([&]{g.change_edge(0,-1,0);});
    expect<std::invalid_argument>([&]{g.change_edge(0,1,2);});
    expect<std::invalid_argument>([&]{(void)g.flow(0,3,-1);});
    check(same(g.edges(),unchanged));
#endif
}
template<class Cap>void limits() {
    const Cap max=std::numeric_limits<Cap>::max();mf_graph<Cap> g(2);
    const int first=g.add_edge(0,1,max),second=g.add_edge(0,1,max);
    check(first==0&&second==1);check(g.flow(0,1)==max);check(g.flow(0,1)==max);check(g.flow(0,1)==0);
    check(g.get_edge(first).cap==max&&g.get_edge(first).flow==max);
    check(g.get_edge(second).cap==max&&g.get_edge(second).flow==max);
    g.change_edge(first,max,0);check(g.flow(0,1,max)==max);check(g.get_edge(first).flow==max);
    const auto cut=g.min_cut(0);check(cut[0]&&!cut[1]);
}
}
int main() {
    primary();limits<int>();limits<I>();limits<std::uint8_t>();limits<std::uint64_t>();
    std::printf("mf_graph ACL API checks=%" PRIu64 "\n",checks);
}
