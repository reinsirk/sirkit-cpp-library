// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_PROJECT_TEST_HEADER
#define SIRKIT_PROJECT_TEST_HEADER "../../../Graph/Project_Selection.cpp"
#endif
#include SIRKIT_PROJECT_TEST_HEADER
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>
std::uint64_t checks=0;
void require(bool okay){++checks;if(!okay){std::cerr<<"sparse reset failure "<<checks<<'\n';std::abort();}}
template<class Cost>void trial(int n,std::uint64_t seed){constexpr int k=7;std::array<int,k>id{0,2,n/4,n/2,n-3,n-2,n-1};TwoVariableSubmodularOpt<Cost> model(n);std::array<std::int64_t,128>energy{};std::array<bool,128>valid{};valid.fill(true);std::mt19937_64 rng(seed);
 auto verify=[&](auto& m){std::int64_t best=INT64_MAX;for(int mask=0;mask<128;++mask)if(valid[mask])best=std::min(best,energy[mask]);if(best==INT64_MAX){bool threw=false;try{(void)m.solve();}catch(const std::domain_error&){threw=true;}require(threw);return;}require(m.solve()==best);require(m.solve()==best);const auto a=m.reconstruct();require(a.size()==std::size_t(n));unsigned mask=0;for(int i=0;i<k;++i)if(a[id[i]])mask|=1u<<i;require(valid[mask]&&energy[mask]==best);};verify(model);
 // 解いたあとに補助頂点を追加し、65536頂点の境界をまたぐ。
 model.add_all_true_profit({id[0],id[2]},Cost{1});for(int mask=0;mask<128;++mask)if((mask&5)==5)--energy[mask];verify(model);
 model.add_all_false_profit({id[1],id[4]},Cost{1});for(int mask=0;mask<128;++mask)if((mask&18)==0)--energy[mask];verify(model);
 for(int step=0;step<(sizeof(Cost)==1?12:32);++step){const int op=int(rng()%13),i=int(rng()%k),j=int(rng()%k);const Cost a=Cost(sizeof(Cost)==1?rng()%2:int(rng()%7)-3),b=Cost(sizeof(Cost)==1?rng()%2:int(rng()%7)-3),d=Cost(sizeof(Cost)==1?rng()%2:int(rng()%7)-3),p=Cost(rng()%3);const Cost c=Cost(std::int64_t(a)+d-b+rng()%2);std::vector<int>subset;unsigned subsetmask=0;for(int q=0;q<k;++q)if(rng()%3==0){subset.push_back(id[q]);subsetmask|=1u<<q;}if(!subset.empty()&&rng()%3==0)subset.push_back(subset.front());const bool value=rng()&1;
 switch(op){case 0:model.add_single_cost(id[i],a,b);break;case 1:model.add_constant(a);break;case 2:model.add_psp_constraint(id[i],id[j]);break;case 3:model.add_psp_penalty(id[i],id[j],p);break;case 4:model.add_both_true_profit(id[i],id[j],p);break;case 5:model.add_both_false_profit(id[i],id[j],p);break;case 6:model.add_submodular_function(id[i],id[j],a,b,c,d);break;case 7:model.add_all_true_profit(subset,p);break;case 8:model.add_all_false_profit(subset,p);break;case 9:if(seed&1)model.force(id[i],value);else model.add_psp_penalty(id[i],id[j],p);break;case 10:if(seed&1)model.add_equality(id[i],id[j]);else model.add_constant(a);break;case 11:model.add_any_true_cost(subset,p);break;case 12:model.add_any_false_cost(subset,p);break;}
 for(int mask=0;mask<128;++mask){const bool x=(mask>>i)&1,y=(mask>>j)&1,alltrue=(unsigned(mask)&subsetmask)==subsetmask,allfalse=(unsigned(mask)&subsetmask)==0;switch(op){case 0:energy[mask]+=x?b:a;break;case 1:energy[mask]+=a;break;case 2:valid[mask]=valid[mask]&&(!x||y);break;case 3:if(x&&!y)energy[mask]+=p;break;case 4:if(x&&y)energy[mask]-=p;break;case 5:if(!x&&!y)energy[mask]-=p;break;case 6:energy[mask]+=x?(y?d:c):(y?b:a);break;case 7:if(alltrue)energy[mask]-=p;break;case 8:if(allfalse)energy[mask]-=p;break;case 9:if(seed&1)valid[mask]=valid[mask]&&(x==value);else if(x&&!y)energy[mask]+=p;break;case 10:if(seed&1)valid[mask]=valid[mask]&&(x==y);else energy[mask]+=a;break;case 11:if(!allfalse)energy[mask]+=p;break;case 12:if(!alltrue)energy[mask]+=p;break;}}
 verify(model);if(step%11==0){auto copy=model;verify(copy);auto moved=std::move(copy);require(copy.variable_count()==0&&copy.solve()==0);verify(moved);auto assigned=TwoVariableSubmodularOpt<Cost>(2);assigned=moved;verify(assigned);swap(moved,assigned);verify(assigned);}
 }
 model.clear();require(model.variable_count()==n&&model.node_count()==n+2&&model.solve()==0);model.add_single_cost(id[0],Cost{0},Cost{-1});require(model.solve()==-1);model.init(n+1);require(model.solve()==0);
}
template<class Cost>void boundary(){const int n=65534;TwoVariableSubmodularOpt<Cost> model(n);model.add_single_cost(0,std::numeric_limits<Cost>::min(),Cost{0});model.force(0,true);require(model.solve()==0&&model.reconstruct()[0]);model.force(0,false);bool threw=false;try{model.solve();}catch(const std::domain_error&){threw=true;}require(threw);model.clear();model.add_single_cost(0,Cost{0},std::numeric_limits<Cost>::max());model.force(0,true);model.add_constant(Cost{1});threw=false;try{model.solve();}catch(const std::overflow_error&){threw=true;}require(threw);model.add_constant(Cost{-1});require(model.solve()==std::numeric_limits<Cost>::max());}
int main(){
#ifdef SIRKIT_PROJECT_SPARSE_TEST_REDUCED
for(int n:{65533,65534,65537})for(int seed=0;seed<2;++seed)
#else
for(int n:{65532,65533,65534,65535,65536,65537,1048576})for(int seed=0;seed<4;++seed)
#endif
{trial<std::int8_t>(n,seed+72);trial<std::int16_t>(n,seed+72);trial<std::int32_t>(n,seed+72);trial<std::int64_t>(n,seed+72);}boundary<std::int8_t>();boundary<std::int16_t>();boundary<std::int32_t>();boundary<std::int64_t>();std::cout<<"PASS checks="<<checks<<'\n';}
