// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_MAP_TEST_SOURCE
#define SIRKIT_MAP_TEST_SOURCE "../../../DataStructure/Hash_map.cpp"
#endif
#ifndef SIRKIT_SET_TEST_SOURCE
#define SIRKIT_SET_TEST_SOURCE "../../../DataStructure/Hash_set.cpp"
#endif
#include SIRKIT_MAP_TEST_SOURCE
#include SIRKIT_SET_TEST_SOURCE
#include SIRKIT_SET_TEST_SOURCE
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <string>
#include <tuple>
#include <vector>
using Inner=HashSet<int>;
using Canon=std::vector<int>;
template<class T>concept PublicHasher=requires{typename T::hasher;};
template<class T>concept PublicEqual=requires{typename T::key_equal;};
static_assert(!PublicHasher<Inner> && !PublicEqual<Inner>);
Canon canonical(const Inner&x){Canon out(x.begin(),x.end());std::sort(out.begin(),out.end());return out;}
Inner make(const Canon&keys,std::uint64_t seed,std::size_t reserve,bool reverse){Inner x(0,GenericHash(seed));x.reserve(reserve);if(reverse){for(auto it=keys.rbegin();it!=keys.rend();++it)x.insert(*it);}else x.insert_range(keys.begin(),keys.end());return x;}
struct ResidueEqual{bool operator()(int a,int b)const noexcept{return (a%10+10)%10==(b%10+10)%10;}};
struct ResidueHash{std::uint64_t operator()(int a)const noexcept{return GenericHash(133)((a%10+10)%10);}};
using ResidueSet=HashSet<int,ResidueHash,ResidueEqual>;
struct OuterResidueHash{std::uint64_t operator()(const ResidueSet&x)const{std::uint64_t sum=0;for(int a:x)sum+=GenericHash(741)((a%10+10)%10);return sum;}};
int main(){
 std::size_t checks=0;
 for(std::uint64_t seed:{UINT64_C(0),UINT64_C(1),UINT64_C(2),UINT64_C(113),UINT64_MAX})for(std::size_t reserve:{std::size_t{0},std::size_t{8},std::size_t{64},std::size_t{1024}}){
  Canon keys;for(int i=-10;i<10;++i)keys.push_back(i);
  auto a=make(keys,1,0,false),b=make(keys,seed,reserve,true);b.rehash(reserve);assert(a==b&&canonical(a)==keys&&canonical(b)==keys);
  for(auto hashseed:{UINT64_C(0),UINT64_C(123),UINT64_MAX}){GenericHash hash(hashseed);assert(hash(a)==hash(b));++checks;}
  HashSet<Inner> outer(0,GenericHash(123));outer.insert(a);assert(outer.contains(b));outer.erase(b,false);assert(outer.empty());++checks;
  HashMap<Inner,int> map(0,GenericHash(18));map[a]=37;assert(map.contains(b)&&map.at(b)==37);map[b]=41;assert(map.size()==1&&map.at(a)==41);++checks;
 }
 std::mt19937_64 rng(1275381);std::vector<Canon>pool(64);
 for(auto&v:pool){std::set<int>values;for(int i=int(rng()%17);i>0;--i)values.insert(int(rng()%61)-30);v.assign(values.begin(),values.end());}
 HashSet<Inner> outer(0,GenericHash(378));HashMap<Inner,int> map(0,GenericHash(379));std::set<Canon>reference;std::map<Canon,int>values;
 for(int rep=0;rep<10000;++rep){const auto&key=pool[rng()%pool.size()];auto a=make(key,rng(),rng()%1024,rng()&1);auto b=make(key,rng(),rng()%1024,rng()&1);assert(a==b&&canonical(a)==key&&canonical(b)==key);GenericHash hash(133);assert(hash(a)==hash(b));
  switch(rng()%7){
   case 0:case 1:{auto got=outer.insert_checked(a);auto want=reference.insert(key);assert(got.second==want.second);map[a]+=1;values[key]+=1;break;}
   case 2:assert(outer.contains(b)==reference.contains(key));assert(map.contains(b)==values.contains(key));if(values.contains(key))assert(map.at(b)==values.at(key));break;
   case 3:outer.erase(b,false);reference.erase(key);map.erase(b,false);values.erase(key);break;
   case 4:if(rep%11==0){outer.rehash(rng()%512);map.rehash(rng()%512);}break;
   case 5:{auto copy=outer;auto copiedmap=map;assert(copy==outer&&copiedmap.size()==map.size());for(const auto&k:reference){auto q=make(k,rng(),0,true);assert(copy.contains(q));}break;}
   case 6:if(rep%23==0){auto moved=std::move(outer);assert(outer.empty());outer=std::move(moved);auto movedmap=std::move(map);assert(map.empty());map=std::move(movedmap);}break;
  }
  assert(outer.size()==reference.size()&&map.size()==values.size());
  if(rep%101==0){std::set<Canon>actual;for(const auto&k:outer)actual.insert(canonical(k));assert(actual==reference);for(const auto&[k,v]:map)assert(values.at(canonical(k))==v);}
  ++checks;
 }
 {
  auto a=make({1,2,3},1,0,false),b=make({1,2,3},2,128,true),c=make({4,5},7,256,false),d=make({4,5},8,1024,true);
  using Deep=HashSet<Inner>;Deep x(0,GenericHash(11)),y(0,GenericHash(12));x.insert(a);x.insert(c);y.insert(d);y.insert(b);assert(x==y&&GenericHash(19)(x)==GenericHash(19)(y));HashSet<Deep>deep;deep.insert(x);assert(deep.contains(y));
  HashSet<std::pair<Inner,int>>pairs;pairs.insert({a,7});assert(pairs.contains({b,7})&&!pairs.contains({b,8}));
  std::vector<Inner>u{a,c},v{b,d},reversed{d,b};assert(u==v&&GenericHash(56)(u)==GenericHash(56)(v));HashMap<std::vector<Inner>,int>vectors;vectors[u]=73;assert(vectors.at(v)==73&&!vectors.contains(reversed));checks+=6;
 }
 {
  HashSet<std::string>a(0,GenericHash(1)),b(0,GenericHash(2));for(auto s:{std::string(),std::string("a\0b",3),std::string(100,'x')}){a.insert(s);b.insert(s);}b.rehash(256);assert(a==b&&GenericHash(7)(a)==GenericHash(7)(b));HashSet<HashSet<std::string>>out;out.insert(a);assert(out.contains(b));checks+=3;
  HashSet<double>a0(0,GenericHash(1)),b0(0,GenericHash(2));a0.insert(+0.0);a0.insert(.5);b0.insert(.5);b0.insert(-0.0);assert(a0==b0&&GenericHash(9)(a0)==GenericHash(9)(b0));++checks;
 }
 {
  ResidueSet a(0,ResidueHash{},ResidueEqual{}),b(0,ResidueHash{},ResidueEqual{});a.insert(17);b.insert(27);assert(a==b);HashSet<ResidueSet,OuterResidueHash>out(0,OuterResidueHash{});out.insert(a);assert(out.contains(b));++checks;
 }
 std::cout<<"PASS "<<checks<<" nested equality/hash/outer lookup cases against canonical std::set/map, permutations/seeds/rehash/erase/copy/move/deep composite keys\n";
}
