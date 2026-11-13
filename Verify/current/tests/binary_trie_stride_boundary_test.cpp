// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <type_traits>
#include <vector>
#ifndef TRIE_SOURCE
#define TRIE_SOURCE "../../../DataStructure/Binary_Trie.cpp"
#endif
#include TRIE_SOURCE
#ifndef TRIE_CLASS
#define TRIE_CLASS BinaryTrie
#endif
static std::uint64_t checks=0, failures=0;
#define REQUIRE(x) do { ++checks; if(!(x)) {std::cerr<<__LINE__<<": "<<#x<<'\n';std::abort();} }while(false)
using Entries=std::vector<std::pair<std::uint64_t,int>>;
static std::multiset<int> ids(const Entries& m,std::uint64_t key) {
 std::multiset<int> out;for(auto[v,id]:m)if(v==key&&id!=-1)out.insert(id);return out;
}
template<class Tr> void validate(const Tr& tr,const Entries& m,std::uint64_t mask,bool exhaustive=false) {
 REQUIRE(tr.size()==m.size());REQUIRE(tr.empty()==m.empty());
 std::vector<std::uint64_t> probes{0,1,mask,mask/2};for(auto[v,id]:m)probes.push_back(v);
 if(exhaustive)for(std::uint64_t x=0;x<=mask;++x)probes.push_back(x);
 std::sort(probes.begin(),probes.end());probes.erase(std::unique(probes.begin(),probes.end()),probes.end());
 std::vector<std::uint64_t> ordered;for(auto[v,id]:m)ordered.push_back(v);std::sort(ordered.begin(),ordered.end());
 struct Held {const std::vector<int>* ref;std::vector<int> copy;};std::vector<Held> held;
 for(auto x:probes){auto r=tr.find(x);std::size_t count=std::count(ordered.begin(),ordered.end(),x);REQUIRE(r.first==count);REQUIRE(tr.count(x)==count);REQUIRE(tr.contains(x)==(count!=0));REQUIRE(std::multiset<int>(r.second.begin(),r.second.end())==ids(m,x));held.push_back({&r.second,r.second});}
 for(auto x:probes){
  std::vector<std::uint64_t> bounds{0,1,2,mask/2,mask,std::numeric_limits<std::uint64_t>::max()};if(mask!=std::numeric_limits<std::uint64_t>::max())bounds.push_back(mask+1);
  if(exhaustive)for(std::uint64_t b=0;b<=mask+1;++b)bounds.push_back(b);
  for(auto b:bounds){std::size_t expected=0;for(auto[v,id]:m)expected+=(v^x)<b;REQUIRE(tr.count_xor_less(x,b)==expected);if(x==0)REQUIRE(tr.count_less(b)==expected);}
  REQUIRE(tr.count_xor_less(x,std::int64_t{-1})==0);REQUIRE(tr.count_xor_less(x,false)==0);
  if(!m.empty()){
   auto lo=std::numeric_limits<std::uint64_t>::max(),hi=std::uint64_t{0};for(auto[v,id]:m){lo=std::min(lo,v^x);hi=std::max(hi,v^x);}
   auto a=tr.min_xor(x),b=tr.max_xor(x);REQUIRE(a.first==lo);REQUIRE(b.first==hi);REQUIRE(std::multiset<int>(a.second.begin(),a.second.end())==ids(m,lo^x));REQUIRE(std::multiset<int>(b.second.begin(),b.second.end())==ids(m,hi^x));
  }
 }
 REQUIRE(tr.count_less(std::int64_t{-1})==0);REQUIRE(tr.count_less(false)==0);
 if(!m.empty()){
  REQUIRE(tr.min_element().first==ordered.front());REQUIRE(tr.max_element().first==ordered.back());
  for(std::size_t k=0;k<m.size();++k){auto r=tr.get_kth(static_cast<std::int64_t>(k+1));REQUIRE(r.first==ordered[k]);REQUIRE(std::multiset<int>(r.second.begin(),r.second.end())==ids(m,ordered[k]));}
 }
 for(const auto& h:held)REQUIRE(*h.ref==h.copy);
 for(auto x:probes){auto a=tr.find(x),b=tr.find(x);REQUIRE(&a.second==&b.second);}
}
static std::size_t nodes(const Entries& m,std::uint64_t add,int bits) {
 std::set<std::pair<int,std::uint64_t>> prefixes;
 for(auto[v,id]:m)for(int shift=0;shift<bits;++shift)prefixes.emplace(shift,v>>shift);
 for(int shift=0;shift<bits;++shift)prefixes.emplace(shift,add>>shift);
 return prefixes.size()+1;
}
template<int L,std::size_t N> void capacity() {
 constexpr std::uint64_t mask=(std::uint64_t{1}<<(L+1))-1;
 TRIE_CLASS<std::uint64_t,L,N> tr;Entries m;
 for(int round=0;round<7;++round){
  for(std::uint64_t key=0;key<=mask;++key){
   const auto x=key^static_cast<std::uint64_t>(round&mask);const int id=static_cast<int>(key%3)-2;
   bool present=std::any_of(m.begin(),m.end(),[&](auto e){return e.first==x;});
   bool expected=nodes(m,x,L+1)>N;bool threw=false;
   try{if(round%2){bool got=tr.insert_unique(x,id);REQUIRE(got==!present);if(got)m.emplace_back(x,id);}else{tr.add(x,id);m.emplace_back(x,id);}}
   catch(const std::length_error&){threw=true;++failures;}
   REQUIRE(threw==(expected&&(!(round%2)||!present)));validate(tr,m,mask,true);
  }
  tr.operate_xor(mask);for(auto&e:m)e.first^=mask;validate(tr,m,mask,true);
  for(std::uint64_t x=0;x<=mask;x+=2){
   for(;;){auto it=std::find_if(m.begin(),m.end(),[&](auto e){return e.first==x;});if(it==m.end())break;
    const int id=it->second;REQUIRE(tr.del(x,id));
    if(id!=-1)m.erase(it);else{
     auto anon=std::find(m.begin(),m.end(),std::pair{x,-1});REQUIRE(anon!=m.end());m.erase(anon);
    }
   }
   REQUIRE(!tr.del(x));validate(tr,m,mask,true);
  }
 }
}
template<int L> void exhaustive() {
 constexpr std::uint64_t mask=(std::uint64_t{1}<<(L+1))-1;
 constexpr std::uint64_t sets=std::uint64_t{1}<<(mask+1);
 for(std::uint64_t subset=0;subset<sets;++subset){
  TRIE_CLASS<std::uint64_t,L,64> tr;Entries m;
  for(std::uint64_t key=0;key<=mask;++key)if(subset>>key&1){tr.add(key,3);m.emplace_back(key,3);tr.add(key,3);m.emplace_back(key,3);tr.add(key);m.emplace_back(key,-1);}
  for(std::uint64_t x=0;x<=mask;++x){tr.operate_xor(x);for(auto&e:m)e.first^=x;validate(tr,m,mask,true);tr.operate_xor(x);for(auto&e:m)e.first^=x;}
 }
}
void wide(){
 constexpr auto hi=std::uint64_t{1}<<63, top=std::numeric_limits<std::uint64_t>::max();
 TRIE_CLASS<std::uint64_t,63,1024> tr;Entries m;
 for(auto key:{std::uint64_t{0},std::uint64_t{1},hi-1,hi,hi+1,top-1,top})for(int id:{-1,7,7,-2}){tr.add(key,id);m.emplace_back(key,id);}
 for(auto x:{hi,hi-1,top,std::uint64_t{1},std::uint64_t{0}}){tr.operate_xor(x);for(auto&e:m)e.first^=x;validate(tr,m,top);}
 for(auto[v,id]:m){REQUIRE(!tr.insert_unique(v,123));}validate(tr,m,top);
}
int main(){
 exhaustive<0>();exhaustive<1>();exhaustive<2>();
 capacity<0,1>();capacity<0,2>();capacity<0,3>();
 capacity<1,2>();capacity<1,3>();capacity<1,4>();capacity<1,6>();capacity<1,7>();
 capacity<2,3>();capacity<2,4>();capacity<2,5>();capacity<2,6>();capacity<2,8>();capacity<2,10>();capacity<2,14>();capacity<2,15>();
 capacity<3,4>();capacity<3,5>();capacity<3,6>();capacity<3,9>();capacity<3,16>();capacity<3,30>();capacity<3,31>();
 capacity<4,5>();capacity<4,6>();capacity<4,7>();capacity<4,32>();capacity<4,62>();capacity<4,63>();wide();
 std::cout<<"{\"checks\":"<<checks<<",\"capacity_failures\":"<<failures<<"}\n";
}
