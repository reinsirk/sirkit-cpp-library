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
struct Case {std::uint64_t initial;int named;int deleted;std::uint64_t lazy;};
int main(){
 // 1514/7659キーで物理プールの未使用末尾を1ノードにする。
 // named=4128はID枠満杯。deleted=2はID付き葉も1個消してID枠を再利用する。
 const Case cases[]={{1514,0,0,0},{1514,0,1,0},{1514,0,1,0x13579bdf2468ace0ULL},
                     {7659,4128,0,0},{7659,4128,1,0x8000000000000000ULL},
                     {7659,4128,2,0x13579bdf2468ace0ULL}};
 std::uint64_t mixed_failures=0, id_recycle_failures=0;
 for(const auto c:cases)for(bool unique:{false,true})for(int budget=0;budget<10;++budget){
  const auto oldlive=live.load();{
   Tr tr;Entries m;
   for(std::uint64_t key=0;key<c.initial;++key){const int id=key<static_cast<std::uint64_t>(c.named)?static_cast<int>(key):-1;tr.add(key,id);m.emplace_back(key,id);}
   // 各4分岐の1葉だけを消すため、祖先を残してfree_listへ4ノードを戻す。
   if(c.deleted){
    const std::uint64_t first=c.initial==1514?0:4132;
    for(std::uint64_t j=0;j<4;++j){const auto key=first+4*j;REQUIRE(tr.del(key));auto it=std::find(m.begin(),m.end(),std::pair{key,-1});REQUIRE(it!=m.end());m.erase(it);}
    if(c.deleted==2){REQUIRE(tr.del(0,0));m.erase(std::find(m.begin(),m.end(),std::pair{std::uint64_t{0},0}));}
   }
   tr.operate_xor(c.lazy);for(auto&e:m)e.first^=c.lazy;auto lazy=c.lazy;
   constexpr auto physical=std::uint64_t{0x9555555555555555ULL};const auto first_key=physical^lazy;
   validate(tr,m);bool threw=false;bool inserted=true;
   fail_after=budget;
   try{if(unique)inserted=tr.insert_unique(first_key,11);else tr.add(first_key,11);}catch(const std::bad_alloc&){threw=true;++exceptions;if(c.deleted)++mixed_failures;if(c.deleted==2)++id_recycle_failures;}
   fail_after=-1;
   if(threw)validate(tr,m);else{REQUIRE(inserted);++successes;m.emplace_back(first_key,11);validate(tr,m);}
   // rollback後に別の物理経路を通し、以前の子参照が混ざらないことを調べる。
   constexpr auto flip=std::uint64_t{0x5a5a5a5a5a5a5a5aULL};tr.operate_xor(flip);for(auto&e:m)e.first^=flip;lazy^=flip;
   constexpr auto other_physical=std::uint64_t{0xeaaaaaaaaaaaaaaaULL};const auto other=other_physical^lazy;
   if(unique)REQUIRE(tr.insert_unique(other,12));else tr.add(other,12);m.emplace_back(other,12);validate(tr,m);
   const auto retry=physical^lazy;
   if(threw){if(unique)REQUIRE(tr.insert_unique(retry,11));else tr.add(retry,11);m.emplace_back(retry,11);}
   else{const auto oldrequests=requests.load();fail_after=0;REQUIRE(!tr.insert_unique(retry,99));fail_after=-1;REQUIRE(requests.load()==oldrequests);}
   validate(tr,m);
   REQUIRE(tr.del(retry,11));m.erase(std::find(m.begin(),m.end(),std::pair{retry,11}));
   REQUIRE(tr.del(other,12));m.erase(std::find(m.begin(),m.end(),std::pair{other,12}));validate(tr,m);
   tr.add(retry);m.emplace_back(retry,-1);validate(tr,m);REQUIRE(tr.del(retry));m.pop_back();validate(tr,m);
  }REQUIRE(live.load()==oldlive);
 }
 REQUIRE(exceptions>0);REQUIRE(mixed_failures>0);REQUIRE(id_recycle_failures>0);REQUIRE(live.load()==0);
 std::cout<<"{\"checks\":"<<checks<<",\"injected_bad_alloc\":"<<exceptions<<",\"successful_operations\":"<<successes<<",\"mixed_source_bad_alloc\":"<<mixed_failures<<",\"recycled_id_bad_alloc\":"<<id_recycle_failures<<",\"live_delta\":"<<live.load()<<"}\n";
}
