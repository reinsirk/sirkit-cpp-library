// SPDX-License-Identifier: CC0-1.0
#ifndef SCC_SOURCE
#define SCC_SOURCE "../../../Graph/SCC.cpp"
#endif
#ifndef TWO_SAT_SOURCE
#define TWO_SAT_SOURCE "../../../Graph/Two_SAT.cpp"
#endif
#include SCC_SOURCE
#include TWO_SAT_SOURCE
#include SCC_SOURCE
#include TWO_SAT_SOURCE
#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>
int main() {
    std::mt19937_64 rng(20261031); std::uint64_t checks=0;
    for(int n=0;n<=35;++n)for(int round=0;round<120;++round){
        scc_graph s(n); Graph g(n); std::vector<std::vector<bool>> reach(n,std::vector<bool>(n));
        for(int i=0;i<n;++i)reach[i][i]=true;
        const int m=n?int(rng()%(n*n+1)):0;
        for(int k=0;k<m;++k){int u=rng()%n,v=rng()%n;s.add_edge(u,v);g[u].push_back(v);reach[u][v]=true;}
        for(int k=0;k<n;++k)for(int i=0;i<n;++i)for(int j=0;j<n;++j)reach[i][j]=reach[i][j]||(reach[i][k]&&reach[k][j]);
        const auto count=s.build();auto groups=s.scc();assert(int(groups.size())==count);std::vector<int> seen(n);
        for(int c=0;c<count;++c)for(int u:groups[c]){assert(s.id(u)==c);++seen[u];++checks;}
        for(int i=0;i<n;++i){assert(seen[i]==1);for(int j=0;j<n;++j){assert((s.id(i)==s.id(j))==(reach[i][j]&&reach[j][i]));++checks;}}
        auto dag=s.condensation(),multi=s.condensation(false);std::vector<std::vector<bool>> expect(count,std::vector<bool>(count));
        for(int u=0;u<n;++u)for(int v:g[u])if(s.id(u)!=s.id(v)){assert(s.id(u)<s.id(v));expect[s.id(u)][s.id(v)]=true;}
        for(int c=0;c<count;++c){std::vector<int> count_to(count);for(int v:dag[c])++count_to[v];for(int t=0;t<count;++t){assert(count_to[t]==int(expect[c][t]));++checks;}for(int v:multi[c])assert(expect[c][v]);}
        scc_graph from(g);assert(from.ids().size()==s.ids().size());for(int i=0;i<n;++i)for(int j=0;j<n;++j)assert((from.id(i)==from.id(j))==(s.id(i)==s.id(j)));
        auto copy=s;copy.reset();assert(copy.size()==0);assert(copy.build()==0);assert(s.size()==n);auto moved=std::move(s);assert(s.size()==0);assert(s.build()==0);assert(moved.component_count()==count);s=moved;assert(s.ids()==moved.ids());s=s;assert(s.ids()==moved.ids());
    }
    for(int n=0;n<=10;++n)for(int round=0;round<600;++round){
        two_sat sat(n);std::vector<std::tuple<int,bool,int,bool>> clauses;
        const int m=n?int(rng()%50):0;
        for(int k=0;k<m;++k){int i=rng()%n,j=rng()%n;bool a=rng()&1,b=rng()&1;switch(rng()%4){
          case 0:sat.add_clause(i,a,j,b);clauses.emplace_back(i,a,j,b);break;
          case 1:sat.imply(i,a,j,b);clauses.emplace_back(i,!a,j,b);break;
          case 2:sat.force(i,a);clauses.emplace_back(i,a,i,a);break;
          default:sat.add_xor(i,j,a);clauses.emplace_back(i,true,j,a);clauses.emplace_back(i,false,j,!a);break;
        }}
        bool possible=false;for(unsigned mask=0;mask<(1u<<n);++mask){bool ok=true;for(auto[i,a,j,b]:clauses)ok &= (bool(mask>>i&1)==a)||(bool(mask>>j&1)==b);possible|=ok;}
        assert(sat.satisfiable()==possible);++checks;
        if(possible)for(auto[i,a,j,b]:clauses){assert((sat.value(i)==a)||(sat.value(j)==b));assert(sat.value(i)==bool(sat.answer()[i]));++checks;}
        auto copy=sat;copy.reset();assert(copy.satisfiable());auto moved=std::move(copy);assert(copy.size()==0);assert(copy.satisfiable());sat=sat;assert(sat.satisfiable()==possible);
        if(n){sat.force(0,true);sat.force(0,false);assert(!sat.satisfiable());}
    }
#ifndef SKIP_RUNTIME_CHECKS
    {scc_graph s(2);bool threw=false;try{(void)s.ids();}catch(const std::logic_error&){threw=true;}assert(threw);threw=false;try{s.add_edge(2,0);}catch(const std::out_of_range&){threw=true;}assert(threw);s.build();s.add_edge(0,1);threw=false;try{(void)s.ids();}catch(const std::logic_error&){threw=true;}assert(threw);}
    {two_sat s(2);bool threw=false;try{(void)s.value(0);}catch(const std::logic_error&){threw=true;}assert(threw);s.satisfiable();s.force(0,true);threw=false;try{(void)s.value(0);}catch(const std::logic_error&){threw=true;}assert(threw);threw=false;try{s.add_clause(2,true,0,false);}catch(const std::out_of_range&){threw=true;}assert(threw);}
#endif
    for(int kind=0;kind<2;++kind){const int n=300000;scc_graph s(n);s.reserve_edges(n);for(int i=1;i<n;++i)s.add_edge(i-1,i);if(kind)s.add_edge(n-1,0);assert(s.build()==(kind?1:n));if(!kind)for(int i=0;i<n;++i)assert(s.id(i)==i);++checks;}
    std::cout<<"SCC/two_sat checks="<<checks<<'\n';
}
