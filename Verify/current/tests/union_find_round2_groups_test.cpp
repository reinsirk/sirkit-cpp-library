// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_UNION_TEST_SOURCE
#define SIRKIT_UNION_TEST_SOURCE "../../../DataStructure/union_find.cpp"
#endif
#include SIRKIT_UNION_TEST_SOURCE
#ifndef SIRKIT_ROLLBACK_UNION_TEST_SOURCE
#define SIRKIT_ROLLBACK_UNION_TEST_SOURCE "../../../DataStructure/rollback-union-find.cpp"
#endif
#include SIRKIT_ROLLBACK_UNION_TEST_SOURCE
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
namespace {
std::size_t checks=0;
void require(bool ok,const char*message){++checks;if(!ok)throw std::runtime_error(message);}
struct Reference {
 std::vector<int> labels;
 std::vector<std::vector<int>> past;
 explicit Reference(int n):labels(n){std::iota(labels.begin(),labels.end(),0);}
 bool merge(int a,int b){past.push_back(labels);a=labels[a];b=labels[b];if(a==b)return false;const auto sa=std::count(labels.begin(),labels.end(),a),sb=std::count(labels.begin(),labels.end(),b);if(sa<sb)std::swap(a,b);for(auto&v:labels)if(v==b)v=a;return true;}
 void rollback(int state){while(static_cast<int>(past.size())>state){labels=std::move(past.back());past.pop_back();}}
 auto groups()const{std::vector<std::vector<int>>out(labels.size());for(int i=0;i<static_cast<int>(labels.size());++i)out[labels[i]].push_back(i);out.erase(std::remove_if(out.begin(),out.end(),[](const auto&v){return v.empty();}),out.end());return out;}
};
template<class UF,bool Rollback>void run_case(int n,int kind,std::mt19937& random){
 UF uf(n);Reference ref(n);std::vector<int> order(n);std::iota(order.begin(),order.end(),0);std::shuffle(order.begin(),order.end(),random);if(kind==5)std::swap(order[0],order[n-1]);
 auto merge=[&](int a,int b){const bool expected=ref.merge(a,b);require(uf.merge(a,b)==expected,"merge result");if constexpr(Rollback)require(uf.get_state()==static_cast<int>(ref.past.size()),"history count");};
 if(kind==0||kind==1){const int c=kind==0?2:4;for(int i=c;i<n;++i)merge(order[i%c],order[i]);}
 else if(kind==2){for(int i=1;i<n;++i)if(i%4)merge(order[i-1],order[i]);}
 else if(kind==3){for(int i=1;i<n;++i)if(random()%5==0)merge(order[i-1],order[i]);}
 else if(kind==4){for(int stride=1;stride<n/2;stride*=2)for(int i=0;i+stride<n;i+=stride*2)merge(order[i],order[i+stride]);}
 else {for(int i=1;i<n;++i)if(i!=n/2)merge(n-1,i-1);}
 auto inspect=[&]{const auto expected=ref.groups();const auto state=ref.past.size();for(int repeat=0;repeat<3;++repeat){const auto actual=uf.groups();require(actual==expected,"group vertices/order");require(uf.component_count()==static_cast<int>(expected.size()),"component count");if constexpr(Rollback)require(uf.get_state()==static_cast<int>(state),"groups changed history");}std::vector<int> counts(n);for(int i=0;i<n;++i)++counts[ref.labels[i]];for(int i=0;i<n;++i){require(uf.root(i)==ref.labels[i],"root representative");require(uf.size(i)==counts[ref.labels[i]],"component size");}};
 inspect();const auto initial=ref;const int state=static_cast<int>(ref.past.size());if constexpr(Rollback)uf.snapshot();
 for(int i=0;i<16;++i){const int a=static_cast<int>(random()%n),b=i%3?a:static_cast<int>(random()%n);merge(a,b);}inspect();
 auto copied=uf;auto copyref=ref;require(copied.groups()==copyref.groups(),"copy groups");copied.merge(0,n-1);require(uf.groups()==ref.groups(),"copy independence");
 if constexpr(Rollback){uf.rollback(state);ref.rollback(state);inspect();uf.rollback();inspect();if(state>0){uf.undo();ref.rollback(state-1);inspect();}uf.rollback(0);ref.rollback(0);inspect();}
 else {uf.reset();ref=Reference(n);inspect();}
 auto moved=std::move(uf);require(uf.empty(),"moved source not empty");require(moved.groups()==ref.groups(),"moved groups");uf.reset(n);require(uf.component_count()==n,"reset moved source");(void)initial;
}
}
int main(){try{std::mt19937 random(893753);for(int n:{255,256,257,263,511,512,513,1024,1025})for(int kind=0;kind<6;++kind){run_case<UnionFind,false>(n,kind,random);run_case<RollbackUnionFind,true>(n,kind,random);}std::cout<<"PASS groups oracle checks="<<checks<<" cases=108, threshold/tail/noncontiguous roots/history\n";}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
