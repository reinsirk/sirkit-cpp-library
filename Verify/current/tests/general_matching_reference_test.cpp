// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_GENERAL_MATCHING_SOURCE
#define SIRKIT_GENERAL_MATCHING_SOURCE "../../../Graph/General_Matching.cpp"
#endif
#include SIRKIT_GENERAL_MATCHING_SOURCE
#include <bit>
#include <cstdlib>
#include <iostream>
#include <random>
static std::uint64_t checks = 0;
void require(bool value) { ++checks; if (!value) { std::cerr << "GeneralMatching check=" << checks << '\n'; std::abort(); } }
template<class F> void rejected(F f) { bool caught=false; try { f(); } catch(const std::exception&) { caught=true; } require(caught); }
template<class T> std::vector<T> saved(std::span<const T> a) { return {a.begin(),a.end()}; }
int optimum(const GeneralMatching& graph) {
    const auto n=static_cast<unsigned>(graph.size()); require(n<=18);
    std::vector<std::uint32_t> neighbors(n);
    for(const auto& [u,v]:graph.edges()) {neighbors[static_cast<std::size_t>(u)]|=std::uint32_t{1}<<v;neighbors[static_cast<std::size_t>(v)]|=std::uint32_t{1}<<u;}
    std::vector<std::uint8_t> dp(std::size_t{1}<<n);
    for(std::uint32_t mask=1;mask<dp.size();++mask) {
        const auto u=std::countr_zero(mask);const auto remaining=mask^(std::uint32_t{1}<<u);auto answer=dp[remaining];
        for(auto candidates=remaining&neighbors[static_cast<std::size_t>(u)];candidates;candidates&=candidates-1) {
            const auto bit=candidates&(~candidates+1);answer=std::max(answer,static_cast<std::uint8_t>(1+dp[remaining^bit]));
        }
        dp[mask]=answer;
    }
    return dp.back();
}
void valid(const GeneralMatching& graph) {
    std::int32_t matched=0;auto pairs=graph.pairs();std::vector<bool> used(static_cast<std::size_t>(graph.size()));
    require(pairs.size()==static_cast<std::size_t>(graph.matching_size()));
    std::int32_t last=-1;
    for(const auto& [u,v]:pairs) {require(u<v&&last<u);last=u;require(!used[static_cast<std::size_t>(u)]&&!used[static_cast<std::size_t>(v)]);used[static_cast<std::size_t>(u)]=used[static_cast<std::size_t>(v)]=true;}
    for(std::int32_t u=0;u<graph.size();++u) {
        const auto v=graph.mate(u);require(graph.partners()[static_cast<std::size_t>(u)]==v);
        if(v==-1)continue;
        require(v>=0&&v<graph.size()&&v!=u&&graph.mate(v)==u);++matched;
        bool edge=false;for(const auto& [a,b]:graph.edges())if((a==u&&b==v)||(a==v&&b==u)){edge=true;break;}require(edge);
    }
    require(matched==2*graph.matching_size());require(graph.is_perfect()==(matched==graph.size()));
}
void solved(GeneralMatching& graph) {
    const auto want=optimum(graph),before=graph.matching_size();const auto* pointer=graph.partners().data();const auto result=graph.solve();
    require(result>=before&&result==want&&graph.is_maximum());require(pointer==graph.partners().data());valid(graph);
    const auto previous=saved(graph.partners());require(graph.solve()==want&&saved(graph.partners())==previous);
}
void exhaustive() {
    for(std::int32_t n=0;n<=6;++n) {
        std::vector<std::pair<std::int32_t,std::int32_t>> possible;
        for(std::int32_t u=0;u<n;++u)for(std::int32_t v=u+1;v<n;++v)possible.emplace_back(u,v);
        for(std::uint32_t mask=0;mask<(std::uint32_t{1}<<possible.size());++mask) {
            GeneralMatching graph(n);
            for(std::size_t i=0;i<possible.size();++i)if(mask&(std::uint32_t{1}<<i))graph.add_edge(possible[i].first,possible[i].second);
            solved(graph);
            if(n&&mask%31==0){const auto before=graph.edge_count();graph.add_edge(0,0);require(graph.is_maximum()&&graph.edge_count()==before);}
        }
    }
}
void random_and_blossoms() {
    std::mt19937_64 random(4551637575ULL);
    for(int trial=0;trial<280;++trial) {
        const auto n=static_cast<std::int32_t>(random()%19);GeneralMatching graph(n);std::vector<std::int32_t> permutation(static_cast<std::size_t>(n));std::iota(permutation.begin(),permutation.end(),std::int32_t{0});std::shuffle(permutation.begin(),permutation.end(),random);
        if(n)for(int i=0;i<45;++i) {
            const auto u=permutation[static_cast<std::size_t>(random()%static_cast<std::uint64_t>(n))],v=permutation[static_cast<std::size_t>(random()%static_cast<std::uint64_t>(n))];graph.add_edge(u,v);
            if(i%11==0)solved(graph);
            if(i%9==0)graph.add_edge(v,u);
            valid(graph);
        }
        solved(graph);graph.clear_matching();require(graph.matching_size()==0);solved(graph);
    }
    // 奇数環を段階的に連結し、各時点で再計算DPと照合。辺順/ラベルを変えて縮約形を変える。
    const std::vector<std::pair<std::int32_t,std::int32_t>> edges{{0,1},{1,2},{2,0},{2,3},{3,4},{4,5},{5,6},{6,2},{4,7},{7,8},{8,9},{9,10},{10,4},{1,11},{11,12},{12,13},{13,14},{14,1},{6,15},{15,16},{16,17},{17,8}};
    for(int round=0;round<32;++round) {
        std::vector<std::int32_t> p(18);std::iota(p.begin(),p.end(),std::int32_t{0});std::shuffle(p.begin(),p.end(),random);auto shuffled=edges;std::shuffle(shuffled.begin(),shuffled.end(),random);GeneralMatching graph(18);
        for(std::size_t i=0;i<shuffled.size();++i) {const auto& [u,v]=shuffled[i];graph.add_edge(p[static_cast<std::size_t>(u)],p[static_cast<std::size_t>(v)]);if(i%5==0)solved(graph);}solved(graph);
    }
}
void ownership() {
    GeneralMatching graph(7);graph.reserve_edges(24);for(auto [u,v]:std::vector<std::pair<int,int>>{{0,1},{1,2},{2,0},{2,3},{3,4},{4,5},{5,6}})graph.add_edge(u,v);solved(graph);
    const auto partners=saved(graph.partners());const auto edges=saved(graph.edges());GeneralMatching copy(graph);copy.add_edge(0,6);solved(copy);require(saved(graph.partners())==partners&&saved(graph.edges())==edges);
    copy=graph;auto* self=&copy;copy=*self;copy=std::move(*self);require(saved(copy.partners())==partners);
    GeneralMatching moved(std::move(copy));require(copy.empty()&&copy.is_maximum()&&copy.is_perfect()&&copy.edge_count()==0&&copy.matching_size()==0);
    copy=std::move(moved);require(moved.empty()&&moved.is_perfect());swap(copy,moved);require(copy.empty());moved.clear_edges();require(moved.size()==7&&moved.edge_count()==0&&moved.is_maximum());moved.reset(3);require(moved.size()==3);moved.clear();require(moved.empty()&&moved.is_perfect());
    GeneralMatching empty;solved(empty);rejected([&]{graph.reserve_edges(std::numeric_limits<std::size_t>::max());});
#ifndef SKIP_RUNTIME_CHECKS
    rejected([&]{GeneralMatching bad(-1);});rejected([&]{graph.add_edge(-1,0);});rejected([&]{graph.add_edge(7,7);});rejected([&]{graph.mate(-1);});rejected([&]{graph.mate(7);});rejected([&]{graph.reset(-2);});require(saved(graph.partners())==partners&&saved(graph.edges())==edges);
#endif
}
void long_path() {
    constexpr std::int32_t n=300000;GeneralMatching graph(n);graph.reserve_edges(static_cast<std::size_t>(n));
    for(std::int32_t i=1;i+1<n;i+=2)graph.add_edge(i,i+1);
    require(graph.solve()==n/2-1);
    for(std::int32_t i=0;i+1<n;i+=2)graph.add_edge(i,i+1);
    require(graph.solve()==n/2&&graph.is_perfect());
    for(std::int32_t i=0;i<n;i+=2)require(graph.mate(i)==i+1&&graph.mate(i+1)==i);
}
int main(){exhaustive();random_and_blossoms();ownership();long_path();std::cout<<"GeneralMatching independent checks="<<checks<<'\n';}
