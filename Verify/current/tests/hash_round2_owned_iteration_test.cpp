// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_MAP_TEST_SOURCE
#define SIRKIT_MAP_TEST_SOURCE "../../../DataStructure/Hash_map.cpp"
#endif
#ifndef SIRKIT_SET_TEST_SOURCE
#define SIRKIT_SET_TEST_SOURCE "../../../DataStructure/Hash_set.cpp"
#endif
#include SIRKIT_MAP_TEST_SOURCE
#include SIRKIT_SET_TEST_SOURCE
#include SIRKIT_MAP_TEST_SOURCE
#include SIRKIT_SET_TEST_SOURCE
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <iostream>
struct Owned {
 static inline int live=0;
 int id=0;std::string text;
 Owned():text("default"){++live;}
 explicit Owned(int x):id(x),text(64,'a'){++live;text+=std::to_string(x);}
 Owned(const Owned&x):id(x.id),text(x.text){++live;}
 Owned(Owned&&x)noexcept:id(x.id),text(std::move(x.text)){++live;}
 ~Owned(){--live;}
 bool operator==(const Owned&x)const{return id==x.id;}
};
struct OwnedHash {std::uint64_t operator()(const Owned&x)const noexcept{return GenericHash(123)(x.id);}};
int main(){
 for(std::size_t reserve:{std::size_t{0},std::size_t{16},std::size_t{1024},std::size_t{65536}})
 for(int n:{0,1,2,3,4,5,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129}){
  assert(Owned::live==0);
  {
   HashMap<std::string,Owned> map(0,GenericHash(123));HashSet<Owned,OwnedHash> set(0,OwnedHash{});
   map.reserve(reserve);set.reserve(reserve);std::map<std::string,int> ref;std::set<int> keys;
   for(int i=0;i<n;++i){auto key=std::string(20,'x')+std::to_string(i);map.try_emplace(key,i);set.emplace(i);ref[key]=i;keys.insert(i);}
   for(int i=0;i<n;i+=3){auto key=std::string(20,'x')+std::to_string(i);map.erase(key,false);set.erase(Owned(i),false);ref.erase(key);keys.erase(i);}
   assert(Owned::live==static_cast<int>(map.size()+set.size()));
   const auto& cm=map;const auto&cs=set;
   static_assert(std::is_same_v<decltype(cm.begin()),decltype(cm.cbegin())>);
   static_assert(std::is_const_v<std::remove_reference_t<decltype(*cm.begin())>>);
   std::vector<std::string> forward,backward;
   for(auto it=cm.begin();it!=cm.end();++it){assert(ref.at(it->first)==it->second.id);forward.push_back(it->first);}
   for(auto first=cm.begin(),it=cm.end();it!=first;){--it;backward.push_back(it->first);}
   std::reverse(backward.begin(),backward.end());assert(forward==backward&&forward.size()==ref.size());
   std::set<int> got;std::vector<int> sf,sb;for(auto it=cs.cbegin();it!=cs.cend();it++){got.insert(it->id);sf.push_back(it->id);}assert(got==keys);
   for(auto first=cs.begin(),it=cs.end();it!=first;){auto before=it--;assert(before!=it);sb.push_back(it->id);}
   std::reverse(sb.begin(),sb.end());assert(sf==sb);
   {auto copy=map;auto setcopy=set;assert(copy.size()==map.size()&&setcopy==set);}
   assert(Owned::live==static_cast<int>(map.size()+set.size()));
   for(auto it=map.begin();it!=map.end();){auto next=it;++next;const auto key=it->first;it=map.erase(it);assert(it==next);ref.erase(key);}
   for(auto it=set.begin();it!=set.end();){auto next=it;++next;const auto id=it->id;it=set.erase(it);assert(it==next);keys.erase(id);}
   assert(ref.empty()&&keys.empty()&&map.empty()&&set.empty()&&Owned::live==0);
#ifndef SKIP_RUNTIME_CHECKS
   bool caught=false;try{auto it=cm.end();--it;}catch(const std::out_of_range&){caught=true;}assert(caught);
   caught=false;try{auto it=cs.end();--it;}catch(const std::out_of_range&){caught=true;}assert(caught);
#endif
  }
  assert(Owned::live==0);
 }
 std::cout<<"PASS owned const/nonconst forward/reverse/erase-next, empty/tombstone/reserved layouts, exact object lifetimes\n";
}
