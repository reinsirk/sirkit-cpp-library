// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <new>
#include <set>
#include <vector>
static std::int64_t fail_after=-1;
static std::atomic<std::size_t> live{0},requests{0};
[[gnu::noinline]]static void* obtain(std::size_t n){++requests;if(fail_after>=0&&fail_after--==0)throw std::bad_alloc();if(auto*p=std::malloc(n?n:1)){++live;return p;}throw std::bad_alloc();}
void* operator new(std::size_t n){return obtain(n);}void* operator new[](std::size_t n){return obtain(n);}
[[gnu::noinline]]static void release(void*p)noexcept{if(p)--live;std::free(p);}
void operator delete(void*p)noexcept{release(p);}void operator delete[](void*p)noexcept{release(p);}void operator delete(void*p,std::size_t)noexcept{release(p);}void operator delete[](void*p,std::size_t)noexcept{release(p);}
#ifndef TRIE_SOURCE
#define TRIE_SOURCE "../../../DataStructure/Binary_Trie.cpp"
#endif
#include TRIE_SOURCE
#ifndef TRIE_CLASS
#define TRIE_CLASS BinaryTrie
#endif
static std::uint64_t checks=0,exceptions=0,successes=0;
#define REQUIRE(x)do{++checks;if(!(x)){std::cerr<<__LINE__<<": "<<#x<<'\n';std::abort();}}while(false)
using Tr=TRIE_CLASS<std::uint64_t,63,1000000>;
using Entries=std::vector<std::pair<std::uint64_t,int>>;
void validate(const Tr& tr,const Entries& m){
 REQUIRE(tr.size()==m.size());REQUIRE(tr.empty()==m.empty());
 std::map<std::uint64_t,std::pair<std::size_t,std::multiset<int>>> expected;
 for(auto[v,id]:m){auto& e=expected[v];++e.first;if(id!=-1)e.second.insert(id);}
 for(auto key:{std::uint64_t{0},std::uint64_t{1},std::uint64_t{1}<<63,std::numeric_limits<std::uint64_t>::max()})expected.try_emplace(key);
 for(const auto&[key,e]:expected){auto got=tr.find(key);REQUIRE(got.first==e.first);REQUIRE(tr.count(key)==e.first);REQUIRE(tr.contains(key)==(e.first!=0));REQUIRE(std::multiset<int>(got.second.begin(),got.second.end())==e.second);}
 if(!m.empty()){
  std::vector<std::uint64_t> ordered;for(auto[v,id]:m)ordered.push_back(v);std::sort(ordered.begin(),ordered.end());
  REQUIRE(tr.min_element().first==ordered.front());REQUIRE(tr.max_element().first==ordered.back());
  for(auto k:{std::size_t{1},m.size()/2+1,m.size()})REQUIRE(tr.get_kth(static_cast<std::int64_t>(k)).first==ordered[k-1]);
  for(auto x:{std::uint64_t{0},std::uint64_t{1}<<63,std::numeric_limits<std::uint64_t>::max()}){
   std::uint64_t lo=std::numeric_limits<std::uint64_t>::max(),hi=0;for(auto[v,id]:m){lo=std::min(lo,v^x);hi=std::max(hi,v^x);}REQUIRE(tr.min_xor(x).first==lo);REQUIRE(tr.max_xor(x).first==hi);
   for(auto bound:{std::uint64_t{0},std::uint64_t{1},std::uint64_t{1}<<63,std::numeric_limits<std::uint64_t>::max()}){std::size_t expected=0;for(auto[v,id]:m)expected+=(v^x)<bound;REQUIRE(tr.count_xor_less(x,bound)==expected);if(x==0)REQUIRE(tr.count_less(bound)==expected);}
  }
 }
}
int main(){
 for(int budget=0;budget<8;++budget){auto oldlive=live.load();fail_after=budget;try{Tr tr;}catch(const std::bad_alloc&){++exceptions;}fail_after=-1;REQUIRE(live.load()==oldlive);}
 // 4128はID枠、7660は初期2048+8192ノードの境界を跨ぐ。
 for(std::uint64_t initial:{0u,31u,32u,33u,4095u,4128u,7660u})for(int mode=0;mode<5;++mode)for(int budget=0;budget<12;++budget){
  const auto oldlive=live.load();{
   Tr tr;Entries m;for(std::uint64_t i=0;i<initial;++i){tr.add(i,static_cast<int>(i));m.emplace_back(i,static_cast<int>(i));}
   constexpr auto lazy=std::uint64_t{1}<<63;tr.operate_xor(lazy);for(auto&e:m)e.first^=lazy;
   const auto key=mode<2?std::numeric_limits<std::uint64_t>::max():lazy+initial;
   if(mode==2){tr.add(key);m.emplace_back(key,-1);}if(mode>=3){for(int id:{7,7,-2,8}){tr.add(key,id);m.emplace_back(key,id);}}
   validate(tr,m);const bool unique=mode==1||mode==4;const bool expected_insert=mode!=4;bool threw=false,inserted=true;
   const auto oldrequests=requests.load();fail_after=budget;
   try{if(unique)inserted=tr.insert_unique(key,11);else tr.add(key,11);}catch(const std::bad_alloc&){threw=true;++exceptions;}fail_after=-1;
   if(mode==4){REQUIRE(!threw);REQUIRE(!inserted);REQUIRE(requests.load()==oldrequests);}
   if(threw){validate(tr,m);if(unique)inserted=tr.insert_unique(key,11);else tr.add(key,11);}else ++successes;
   REQUIRE(inserted==expected_insert);if(inserted)m.emplace_back(key,11);validate(tr,m);
   if(inserted){REQUIRE(tr.del(key,11));m.erase(std::find(m.begin(),m.end(),std::pair{key,11}));}validate(tr,m);
   // 読み取りと不存在削除は確保しない。
   const auto prior=requests.load();fail_after=0;
   REQUIRE(tr.count(key)==tr.find(key).first);REQUIRE(tr.contains(key)==(tr.count(key)!=0));REQUIRE(!tr.del(std::uint64_t{123456789}));
   REQUIRE(tr.count_less(std::uint64_t{0})==0);REQUIRE(tr.count_xor_less(key,std::uint64_t{0})==0);
   if(!tr.empty()){(void)tr.min_element();(void)tr.max_element();(void)tr.min_xor(key);(void)tr.max_xor(key);(void)tr.get_kth(1);}
   fail_after=-1;REQUIRE(requests.load()==prior);
  }REQUIRE(live.load()==oldlive);
 }
 REQUIRE(live.load()==0);std::cout<<"{\"checks\":"<<checks<<",\"injected_bad_alloc\":"<<exceptions<<",\"successful_operations\":"<<successes<<",\"live_delta\":"<<live.load()<<"}\n";
}
