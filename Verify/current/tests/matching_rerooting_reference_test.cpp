// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef MATCHING_SOURCE
#define MATCHING_SOURCE "../../../Graph/Bipartite_Matching.cpp"
#endif
#ifndef REROOTING_SOURCE
#define REROOTING_SOURCE "../../../Graph/Rerooting_DP.cpp"
#endif
#include MATCHING_SOURCE
#include REROOTING_SOURCE
#include MATCHING_SOURCE
#include REROOTING_SOURCE
#include <cassert>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <string>

static std::mt19937_64 random_engine(0xac3921bd);
static std::uint64_t checks=0;
static void check(bool ok){++checks;assert(ok);}
static int random_int(int n){return static_cast<int>(random_engine()%static_cast<unsigned>(n));}
template<class F>static void rejects(F f){bool thrown=false;try{f();}catch(const std::exception&){thrown=true;}check(thrown);}

int matching_oracle(const std::vector<std::vector<bool>>& graph,int right){
    std::vector<int> states(std::size_t{1}<<right,-1);states[0]=0;
    for(const auto& row:graph){auto next=states;for(std::size_t mask=0;mask<states.size();++mask)if(states[mask]>=0)for(int v=0;v<right;++v)if(row[static_cast<std::size_t>(v)]&&!(mask>>v&1u))next[mask|(std::size_t{1}<<v)]=std::max(next[mask|(std::size_t{1}<<v)],states[mask]+1);states.swap(next);}
    return *std::max_element(states.begin(),states.end());
}
void matching_validate(BipartiteMatching& matching,const std::vector<std::vector<bool>>& graph,int right,int optimum){
    const int left=static_cast<int>(graph.size());
    check(matching.left_size()==left&&matching.right_size()==right&&matching.matching_size()==optimum);
    check(matching.is_maximum());check(matching.solve()==optimum);
    std::vector<bool> seen_left(static_cast<std::size_t>(left)),seen_right(static_cast<std::size_t>(right));
    for(auto[u,v]:matching.pairs()){check(u>=0&&u<left&&v>=0&&v<right);check(graph[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)]);check(!seen_left[static_cast<std::size_t>(u)]&&!seen_right[static_cast<std::size_t>(v)]);seen_left[static_cast<std::size_t>(u)]=true;seen_right[static_cast<std::size_t>(v)]=true;check(matching.match_left(u)==v&&matching.match_right(v)==u);}
    check(matching.pairs().size()==static_cast<std::size_t>(optimum));
    for(int u=0;u<left;++u)check(matching.left_matches()[static_cast<std::size_t>(u)]==matching.match_left(u));
    auto cover=matching.min_vertex_cover();auto independent=matching.max_independent_set();
    check(cover.size()==static_cast<std::size_t>(optimum));check(independent.size()==static_cast<std::size_t>(left+right-optimum));
    std::vector<bool> covered_left(static_cast<std::size_t>(left)),covered_right(static_cast<std::size_t>(right)),ind_left(static_cast<std::size_t>(left)),ind_right(static_cast<std::size_t>(right));
    for(int u:cover.left){check(!covered_left[static_cast<std::size_t>(u)]);covered_left[static_cast<std::size_t>(u)]=true;}
    for(int v:cover.right){check(!covered_right[static_cast<std::size_t>(v)]);covered_right[static_cast<std::size_t>(v)]=true;}
    for(int u:independent.left)ind_left[static_cast<std::size_t>(u)]=true;
    for(int v:independent.right)ind_right[static_cast<std::size_t>(v)]=true;
    for(int u=0;u<left;++u){check(covered_left[static_cast<std::size_t>(u)]!=ind_left[static_cast<std::size_t>(u)]);for(int v=0;v<right;++v)if(graph[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)]){check(covered_left[static_cast<std::size_t>(u)]||covered_right[static_cast<std::size_t>(v)]);check(!(ind_left[static_cast<std::size_t>(u)]&&ind_right[static_cast<std::size_t>(v)]));}}
    for(int v=0;v<right;++v)check(covered_right[static_cast<std::size_t>(v)]!=ind_right[static_cast<std::size_t>(v)]);
    // 別の反復交互路探索で、残された増加路がないことを確認。
    std::vector<int> queue;
    std::fill(seen_left.begin(),seen_left.end(),false);std::fill(seen_right.begin(),seen_right.end(),false);
    for(int u=0;u<left;++u)if(matching.match_left(u)==-1){queue.push_back(u);seen_left[static_cast<std::size_t>(u)]=true;}
    for(std::size_t head=0;head<queue.size();++head){int u=queue[head];for(int v=0;v<right;++v)if(graph[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)]&&matching.match_left(u)!=v&&!seen_right[static_cast<std::size_t>(v)]){seen_right[static_cast<std::size_t>(v)]=true;int w=matching.match_right(v);check(w!=-1);if(!seen_left[static_cast<std::size_t>(w)]){seen_left[static_cast<std::size_t>(w)]=true;queue.push_back(w);}}}
}
void matching_tests(){
    for(int left=0;left<=4;++left)for(int right=0;right<=4;++right){
        const unsigned total=1u<<(left*right);
        for(unsigned mask=0;mask<total;++mask){BipartiteMatching matching(left,right);std::vector<std::vector<bool>> graph(static_cast<std::size_t>(left),std::vector<bool>(static_cast<std::size_t>(right)));std::size_t edges=0;for(int u=0;u<left;++u)for(int v=0;v<right;++v)if(mask>>(u*right+v)&1u){matching.add_edge(u,v);graph[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)]=true;++edges;}check(matching.edge_count()==edges);int optimum=matching_oracle(graph,right);check(matching.solve()==optimum);matching_validate(matching,graph,right,optimum);}
    }
    for(int repeat=0;repeat<700;++repeat){int left=random_int(10),right=random_int(11);BipartiteMatching matching(left,right);std::vector<std::vector<bool>> graph(static_cast<std::size_t>(left),std::vector<bool>(static_cast<std::size_t>(right)));for(int step=0;step<60;++step){if(left&&right){int u=random_int(left),v=random_int(right);matching.reserve_neighbors(u,matching.neighbors(u).size()+1);matching.add_edge(u,v);graph[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)]=true;}int optimum=matching_oracle(graph,right);check(matching.solve()==optimum);matching_validate(matching,graph,right,optimum);if(step%17==0){auto copied=matching;matching_validate(copied,graph,right,optimum);copied.clear_matching();check(copied.matching_size()==0);check(copied.solve()==optimum);auto moved=std::move(copied);check(copied.empty());check(moved.solve()==optimum);copied=moved;check(copied.solve()==optimum);moved=std::move(copied);check(copied.empty());swap(moved,copied);check(moved.empty());check(copied.solve()==optimum);copied.clear_edges();check(copied.solve()==0);copied.clear();check(copied.empty());copied.reset(left,right);check(copied.solve()==0);}}
    }
#ifndef SKIP_RUNTIME_CHECKS
    BipartiteMatching checked(2,3);rejects([&]{checked.add_edge(-1,0);});rejects([&]{checked.add_edge(0,3);});checked.add_edge(0,0);rejects([&]{checked.min_vertex_cover();});checked.solve();checked.add_edge(1,1);rejects([&]{checked.max_independent_set();});
#endif
    // 最後の1本のために左右30万頂点を通る増加路をたどる。
    constexpr int n=300000;BipartiteMatching chain(n,n);for(int u=0;u<n-1;++u)chain.add_edge(u,u+1);check(chain.solve()==n-1);for(int u=0;u<n;++u)chain.add_edge(u,u);check(chain.solve()==n);for(int u=0;u<n;++u)check(chain.match_left(u)==u);check(chain.min_vertex_cover().size()==n);
}

using Sum=std::pair<std::int64_t,std::int64_t>;
Sum merge_sum(const Sum& a,const Sum& b){return{a.first+b.first,a.second+b.second};}
void rerooting_case(int n,const std::vector<std::pair<int,int>>& edges){
    RerootingDP<Sum> dp(n);RerootingDP<std::string> text(n);
    std::vector<std::vector<std::pair<int,std::size_t>>> graph(static_cast<std::size_t>(n));
    for(std::size_t id=0;id<edges.size();++id){auto[u,v]=edges[id];check(dp.add_edge(u,v)==id);text.add_edge(u,v);graph[static_cast<std::size_t>(u)].emplace_back(v,id);graph[static_cast<std::size_t>(v)].emplace_back(u,id);}
    auto vertex=[](const Sum& value,int){return Sum{value.first,value.second+1};};
    auto edge=[](const Sum& value,int from,int to,std::size_t id){return Sum{value.first+value.second*static_cast<std::int64_t>(1+(id*17+static_cast<unsigned>(from)*3+static_cast<unsigned>(to))%9),value.second};};
    auto join=[](const std::string& a,const std::string& b){return a+b;};
    auto vertex_text=[](const std::string& a,int v){return std::to_string(v)+"["+a+"]";};
    auto edge_text=[](const std::string& a,int from,int to,std::size_t id){return "{"+std::to_string(id)+":"+std::to_string(from)+">"+std::to_string(to)+a+"}";};
    dp.build(Sum{},merge_sum,vertex,edge);text.build(std::string{},join,vertex_text,edge_text);
    auto naive_sum=[&](auto&& self,int v,std::size_t excluded)->Sum{Sum a{};for(auto[u,id]:graph[static_cast<std::size_t>(v)])if(id!=excluded)a=merge_sum(a,edge(self(self,u,id),u,v,id));return vertex(a,v);};
    auto naive_text=[&](auto&& self,int v,std::size_t excluded)->std::string{std::string a;for(auto[u,id]:graph[static_cast<std::size_t>(v)])if(id!=excluded)a+=edge_text(self(self,u,id),u,v,id);return vertex_text(a,v);};
    const auto absent=std::numeric_limits<std::size_t>::max();
    check(dp.has_results()&&dp.values().size()==static_cast<std::size_t>(n));check(dp.component_count()==static_cast<std::size_t>(n)-edges.size());
    for(int v=0;v<n;++v){check(dp[v]==naive_sum(naive_sum,v,absent));check(text[v]==naive_text(naive_text,v,absent));check(dp.neighbors(v).size()==graph[static_cast<std::size_t>(v)].size());for(std::size_t k=0;k<graph[static_cast<std::size_t>(v)].size();++k){auto[to,id]=graph[static_cast<std::size_t>(v)][k];auto arc=dp.neighbors(v)[k];check(arc/2==id);check((arc&1?edges[id].first:edges[id].second)==to);}}
    for(std::size_t id=0;id<edges.size();++id){auto[u,v]=edges[id];check(dp.edge(id)==edges[id]);check(dp.message(id)==edge(naive_sum(naive_sum,u,id),u,v,id));check(dp.message(id,true)==edge(naive_sum(naive_sum,v,id),v,u,id));check(text.message(id)==edge_text(naive_text(naive_text,u,id),u,v,id));check(text.message(id,true)==edge_text(naive_text(naive_text,v,id),v,u,id));}
    auto copied=dp;auto old=std::vector<Sum>(dp.values().begin(),dp.values().end());copied.clear();check(copied.empty());check(std::vector<Sum>(dp.values().begin(),dp.values().end())==old);copied=dp;auto moved=std::move(copied);check(copied.empty());check(moved.values().size()==old.size());copied.reset(n);check(!copied.has_results());copied=moved;moved=std::move(copied);check(copied.empty());swap(moved,copied);check(moved.empty());check(std::vector<Sum>(copied.values().begin(),copied.values().end())==old);
    for(int kind=0;kind<3;++kind){int calls=0;auto throwing_merge=[&](const Sum&a,const Sum&b){if(kind==0&&++calls==2)throw std::runtime_error("merge");return merge_sum(a,b);};auto throwing_vertex=[&](const Sum&a,int v){if(kind==1&&++calls==2)throw std::runtime_error("vertex");return vertex(a,v);};auto throwing_edge=[&](const Sum&a,int u,int v,std::size_t id){if(kind==2&&++calls==2)throw std::runtime_error("edge");return edge(a,u,v,id);};try{dp.build(Sum{},throwing_merge,throwing_vertex,throwing_edge);}catch(const std::runtime_error&){check(std::vector<Sum>(dp.values().begin(),dp.values().end())==old);}}
}
struct NoDefault{int value;NoDefault()=delete;explicit NoDefault(int v):value(v){};};
void rerooting_tests(){
    for(int n=0;n<=12;++n)for(int repeat=0;repeat<130;++repeat){std::vector<int>labels(static_cast<std::size_t>(n));std::iota(labels.begin(),labels.end(),0);std::shuffle(labels.begin(),labels.end(),random_engine);std::vector<std::pair<int,int>> edges;for(int v=1;v<n;++v)if(random_int(4)){int p=random_int(v);auto u=labels[static_cast<std::size_t>(v)],w=labels[static_cast<std::size_t>(p)];if(random_int(2))std::swap(u,w);edges.emplace_back(u,w);}std::shuffle(edges.begin(),edges.end(),random_engine);rerooting_case(n,edges);}
    RerootingDP<NoDefault> nd(2);nd.add_edge(0,1);nd.build(NoDefault(0),[](const NoDefault&a,const NoDefault&b){return NoDefault(a.value+b.value);},[](const NoDefault&a,int){return NoDefault(a.value+1);},[](const NoDefault&a,int,int,std::size_t){return a;});check(nd[0].value==2&&nd[1].value==2);
#ifndef SKIP_RUNTIME_CHECKS
    RerootingDP<int> bad(3);rejects([&]{bad.add_edge(0,3);});rejects([&]{bad.add_edge(1,1);});rejects([&]{bad.values();});bad.add_edge(0,1);bad.add_edge(1,2);bad.add_edge(2,0);rejects([&]{bad.build(0,std::plus<int>{},[](int a,int){return a+1;},[](int a,int,int,std::size_t){return a;});});RerootingDP<int> parallel(2);parallel.add_edge(0,1);parallel.add_edge(1,0);rejects([&]{parallel.build(0,std::plus<int>{},[](int a,int){return a;},[](int a,int,int,std::size_t){return a;});});
#endif
    constexpr int n=300000;for(bool chain:{false,true}){RerootingDP<Sum> dp(n);dp.reserve_edges(n-1);for(int v=1;v<n;++v)dp.add_edge(chain?v-1:0,v);dp.build(Sum{},merge_sum,[](const Sum&a,int){return Sum{a.first,a.second+1};},[](const Sum&a,int,int,std::size_t){return Sum{a.first+a.second,a.second};});for(int v:{0,1,n/2,n-1}){const std::int64_t left=v,right=n-1-v;const std::int64_t wanted=chain?left*(left+1)/2+right*(right+1)/2:(v==0?n-1:2*std::int64_t(n)-3);check(dp[v].first==wanted&&dp[v].second==n);}}
}
int main(){matching_tests();rerooting_tests();std::cout<<"matching/rerooting independent checks="<<checks<<'\n';}
