// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_ODC_SOURCE
#define SIRKIT_ODC_SOURCE "../../../Graph/Offline_Dynamic_Connectivity.cpp"
#endif
#ifndef SIRKIT_FG_SOURCE
#define SIRKIT_FG_SOURCE "../../../Graph/Functional_Graph.cpp"
#endif
#include SIRKIT_ODC_SOURCE
#include SIRKIT_FG_SOURCE
#include <array>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <tuple>

std::uint64_t checks = 0;
void require(bool value) { ++checks; if (!value) { std::cerr << "failed check " << checks << '\n'; std::abort(); } }
template<class F> void rejected(F f) { bool caught = false; try { f(); } catch (const std::exception&) { caught = true; } require(caught); }
std::mt19937_64 random_engine(178514349005ULL);
std::size_t rng(std::size_t bound) { return static_cast<std::size_t>(random_engine() % bound); }
void functional_case(const std::vector<int>& next) {
    const int n = static_cast<int>(next.size()); FunctionalGraph g(next);
    require(g.size() == n); require(g.empty() == !n); require(std::equal(next.begin(),next.end(),g.successors().begin()));
    std::vector<int> seen_cycle(n), cycle_representatives;
    for (int id = 0; id < g.cycle_count(); ++id) {
        const auto cycle = g.cycle(id); require(!cycle.empty());
        for (std::size_t i = 0; i < cycle.size(); ++i) {
            const int v = cycle[i]; require(next[v] == cycle[(i+1)%cycle.size()]); require(!seen_cycle[v]++);
            require(g.on_cycle(v));require(g.info(v).id==id);require(g.info(v).position==static_cast<int>(i));
        }
        cycle_representatives.push_back(cycle[0]);
    }
    for (int from = 0; from < n; ++from) {
        std::vector<int> time(n,-1), path; int v=from;
        while (time[v] == -1) { time[v]=static_cast<int>(path.size());path.push_back(v);v=next[v]; }
        const int depth=time[v], length=static_cast<int>(path.size())-depth;
        const auto info=g.info(from);
        require(info.depth==depth);require(info.entry==v);require(info.length==length);require(g.on_cycle(from)==(depth==0));
        require(g.cycle(info.id)[info.position]==v);require(g.successor(from)==next[from]);
        const std::array<std::uint64_t,10> ks{0,1,2,3,static_cast<std::uint64_t>(n),static_cast<std::uint64_t>(n*2+1),std::uint64_t{1}<<32,std::uint64_t{1}<<63,std::numeric_limits<std::uint64_t>::max(),random_engine()};
        for (const auto k:ks) {
            const auto index=k<path.size()?static_cast<std::size_t>(k):static_cast<std::size_t>(depth+(k-static_cast<std::uint64_t>(depth))%static_cast<std::uint64_t>(length));
            require(g.kth_successor(from,k)==path[index]);
        }
        int component=0;
        for (int to=0;to<n;++to) {
            require(g.distance(from,to)==time[to]);require(g.reachable(from,to)==(time[to]>=0));
            // 弱連結は各頂点を直接たどった周期の最小ラベルで独立判定。
            std::vector<int> t(n,-1); int x=to, step=0;
            while(t[x]<0){t[x]=step++;x=next[x];}
            int minimum=x;for(int y=next[x];y!=x;y=next[y])minimum=std::min(minimum,y);
            int own=v;for(int y=next[v];y!=v;y=next[y])own=std::min(own,y);
            component+=(minimum==own);
        }
        require(g.component_size(from)==component);
    }
    if(n) {
        auto copy=g;auto saved=g.successors().data();copy.build(copy.successors());require(copy.successor(0)==next[0]);require(g.successors().data()==saved);
        auto moved=std::move(copy);require(copy.empty());require(copy.cycle_count()==0);copy.build(next);require(copy.successor(0)==next[0]);
        copy=moved;auto* alias=&copy;copy=*alias;copy=std::move(*alias);require(copy.successor(0)==next[0]);moved.clear();require(moved.empty());swap(copy,moved);require(copy.empty());require(moved.size()==n);
    }
}
void functional_tests() {
    functional_case({});
    for(int n=1;n<=5;++n){std::uint64_t total=1;for(int i=0;i<n;++i)total*=static_cast<unsigned>(n);for(std::uint64_t mask=0;mask<total;++mask){auto code=mask;std::vector<int> next(n);for(auto& v:next){v=static_cast<int>(code%static_cast<unsigned>(n));code/=static_cast<unsigned>(n);}functional_case(next);}}
    for(int trial=0;trial<350;++trial){const int n=static_cast<int>(rng(65)+1);std::vector<int> next(n);for(auto& v:next)v=static_cast<int>(rng(static_cast<std::size_t>(n)));functional_case(next);}
    const int n=300000;std::vector<int> next(n);std::iota(next.begin(),next.end(),1);next.back()=n-1;
    FunctionalGraph chain(next);require(chain.info(0).depth==n-1);require(chain.kth_successor(0,n-2)==n-2);require(chain.kth_successor(0,std::numeric_limits<std::uint64_t>::max())==n-1);require(chain.distance(0,n-1)==n-1);require(chain.distance(n-1,0)==-1);
    std::iota(next.begin(),next.end(),1);next.back()=0;chain.build(next);require(chain.info(12345).length==n);require(chain.kth_successor(12345,std::numeric_limits<std::uint64_t>::max())==static_cast<int>((12345+std::numeric_limits<std::uint64_t>::max()%static_cast<unsigned>(n))%static_cast<unsigned>(n)));
    std::fill(next.begin(),next.end(),0);chain.build(next);require(chain.component_size(299999)==n);require(chain.distance(299999,1)==-1);require(chain.distance(299999,0)==1);
#ifndef SKIP_RUNTIME_CHECKS
    rejected([&]{chain.successor(-1);});rejected([&]{chain.kth_successor(n,0);});rejected([&]{chain.distance(0,n);});rejected([&]{chain.cycle(-1);});rejected([&]{chain.cycle(chain.cycle_count());});
    const auto saved=chain.successors().data();next[123]=n;rejected([&]{chain.build(next);});require(saved==chain.successors().data());require(chain.successor(123)==0);
#endif
}
struct Naive {
    struct E {int u,v;bool active;};
    std::vector<std::int64_t> weights;std::vector<E> edges;
    std::vector<int> components() const {
        const int n=static_cast<int>(weights.size());std::vector<int> c(n,-1), stack;int id=0;
        for(int start=0;start<n;++start)if(c[start]<0){c[start]=id;stack.push_back(start);while(!stack.empty()){int v=stack.back();stack.pop_back();for(const auto e:edges)if(e.active){int to=e.u==v?e.v:e.v==v?e.u:-1;if(to>=0&&c[to]<0){c[to]=id;stack.push_back(to);}}}++id;}
        return c;
    }
};
void connectivity_tests() {
    OfflineDynamicConnectivity empty;require(empty.empty());require(empty.solve().empty());empty.query_component_count();require(empty.solve()==std::vector<std::int64_t>{0});empty.clear();require(empty.query_count()==0);
    for(int trial=0;trial<1100;++trial){
        const int n=static_cast<int>(rng(18)+1);Naive naive;naive.weights.resize(n);for(auto& x:naive.weights)x=static_cast<std::int64_t>(rng(81))-40;
        OfflineDynamicConnectivity g(naive.weights);g.reserve_edges(16);g.reserve_queries(16);g.reserve_additions(16);std::vector<std::int64_t> expected;std::size_t additions=0;
        for(int step=0;step<250;++step){const int u=static_cast<int>(rng(static_cast<std::size_t>(n))),v=static_cast<int>(rng(static_cast<std::size_t>(n)));const auto kind=rng(9);
            if(kind<2){const auto id=g.add_edge(u,v);require(id==naive.edges.size());naive.edges.push_back({u,v,true});}
            else if(kind==2){std::vector<std::size_t> ids;for(std::size_t i=0;i<naive.edges.size();++i)if(naive.edges[i].active)ids.push_back(i);if(!ids.empty()){auto id=ids[rng(ids.size())];g.remove_edge(id);naive.edges[id].active=false;}}
            else if(kind==3){auto delta=static_cast<std::int64_t>(rng(201))-100;g.add_vertex(u,delta);naive.weights[u]+=delta;++additions;}
            else {const auto c=naive.components();std::size_t id=0;std::int64_t value=0;
                if(kind==4){id=g.query_same(u,v);value=c[u]==c[v];}
                if(kind==5){id=g.query_size(u);value=std::count(c.begin(),c.end(),c[u]);}
                if(kind==6){id=g.query_component_count();value=*std::max_element(c.begin(),c.end())+1;}
                if(kind>=7){id=g.query_component_sum(u);for(int x=0;x<n;++x)if(c[x]==c[u])value+=naive.weights[x];}
                require(id==expected.size());expected.push_back(value);
            }
            if(step%79==0)require(g.solve()==expected);
        }
        require(g.solve()==expected);require(g.solve()==expected);require(g.query_count()==expected.size());require(g.edge_count()==naive.edges.size());require(g.addition_count()==additions);
        auto copy=g;auto original=g.solve();copy.add_vertex(0,125);copy.query_component_sum(0);require(g.solve()==original);auto moved=std::move(copy);require(copy.empty());require(copy.edge_count()==0);require(copy.query_count()==0);copy.query_component_count();require(copy.solve()==std::vector<std::int64_t>{0});copy=std::move(moved);require(moved.empty());require(copy.query_count()==expected.size()+1);copy=g;auto* alias=&copy;copy=*alias;copy=std::move(*alias);require(copy.solve()==expected);copy.clear_history();require(copy.size()==n);require(copy.edge_count()==0);copy.query_component_sum(0); // 初期値へ戻る。
        OfflineDynamicConnectivity initial(n);require(initial.size()==n);initial.reset(std::span<const std::int64_t>(naive.weights));initial.query_component_sum(0);require(initial.solve()[0]==naive.weights[0]);swap(initial,copy);copy.clear();require(copy.empty());
    }
    // 加算の発生時刻と辺区間の分割順が異なる場合も全照会時の重みを保持する。
    OfflineDynamicConnectivity g(4);g.add_vertex(0,7);auto a=g.add_edge(0,1);g.add_vertex(1,11);g.query_component_sum(0);auto b=g.add_edge(1,2);g.query_component_sum(2);g.remove_edge(a);g.add_vertex(0,-5);g.query_component_sum(0);g.query_component_sum(2);g.remove_edge(b);g.query_component_sum(1);require(g.solve()==std::vector<std::int64_t>({18,18,2,11,11}));
    const int n=300000;g.reset(n);std::vector<std::size_t> ids;ids.reserve(n-1);for(int i=0;i<n-1;++i)ids.push_back(g.add_edge(i,i+1));g.query_size(0);for(int i=0;i<n-1;++i){g.remove_edge(ids[i]);g.query_component_count();}auto values=g.solve();require(values[0]==n);for(int i=1;i<n;++i)require(values[static_cast<std::size_t>(i)]==i+1);
#ifndef SKIP_RUNTIME_CHECKS
    rejected([&]{g.add_edge(-1,0);});rejected([&]{g.add_vertex(n,1);});rejected([&]{g.query_same(0,n);});rejected([&]{g.query_size(n);});rejected([&]{g.query_component_sum(-1);});rejected([&]{g.remove_edge(ids[0]);});rejected([&]{g.remove_edge(g.edge_count());});
#endif
    rejected([&]{g.reset(-1);});require(g.size()==n);
}
int main(){functional_tests();connectivity_tests();std::cout<<"connectivity/functional independent checks="<<checks<<'\n';}
