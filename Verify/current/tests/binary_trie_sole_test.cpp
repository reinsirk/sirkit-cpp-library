// SPDX-License-Identifier: CC0-1.0
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>
#ifndef TRIE_SOURCE
#define TRIE_SOURCE "../../../DataStructure/Binary_Trie.cpp"
#endif
#include TRIE_SOURCE
#ifndef TRIE_CLASS
#define TRIE_CLASS BinaryTrie
#endif
static std::uint64_t checks=0;
#define REQUIRE(x)do{++checks;if(!(x)){std::cerr<<__LINE__<<": "<<#x<<'\n';std::abort();}}while(false)
template<class T,int L>void sole(){
 constexpr std::uint64_t mask=[] {if constexpr(L==63)return std::numeric_limits<std::uint64_t>::max();else return (std::uint64_t{1}<<(L+1))-1;}();
 const std::array<std::uint64_t,6> values{0,1,mask,mask/2,mask/3,mask^(mask/3)};
 for(auto initial:values){
  TRIE_CLASS<T,L,128> tr;tr.add(static_cast<T>(initial),42);auto value=initial;int current_id=42;
  auto check=[&]{
   REQUIRE(tr.size()==1);REQUIRE(tr.min_element().first==value);REQUIRE(tr.max_element().first==value);REQUIRE(tr.get_kth(1).first==value);
   for(auto x:values){
    auto lo=tr.min_xor(static_cast<T>(x)),hi=tr.max_xor(static_cast<T>(x));REQUIRE(lo.first==(value^x));REQUIRE(hi.first==(value^x));
    REQUIRE(lo.second==std::vector<int>{current_id});REQUIRE(hi.second==std::vector<int>{current_id});
    REQUIRE(&lo.second==&tr.find(static_cast<T>(value)).second);REQUIRE(&hi.second==&lo.second);
    for(auto bound:values){REQUIRE(tr.count_xor_less(static_cast<T>(x),bound)==static_cast<std::size_t>((value^x)<bound));if(x==0)REQUIRE(tr.count_less(bound)==static_cast<std::size_t>(value<bound));}
   }
   REQUIRE(!tr.insert_unique(static_cast<T>(value),99));REQUIRE(tr.find(static_cast<T>(value)).second==std::vector<int>{current_id});
  };
  for(auto lazy:values){tr.operate_xor(static_cast<T>(lazy));value^=lazy;check();}
  tr.add(static_cast<T>(value),5);REQUIRE(tr.size()==2);REQUIRE(tr.min_xor(static_cast<T>(mask)).first==(value^mask));REQUIRE(tr.del(static_cast<T>(value),42));current_id=5;check();
  REQUIRE(tr.del(static_cast<T>(value),5));REQUIRE(tr.empty());tr.add(static_cast<T>(initial),42);value=initial;current_id=42;check();
 }
}
int main(){sole<std::uint8_t,0>();sole<std::uint8_t,1>();sole<std::uint8_t,2>();sole<std::int8_t,6>();sole<std::uint16_t,15>();sole<int,30>();sole<std::uint32_t,31>();sole<std::int64_t,62>();sole<std::uint64_t,63>();std::cout<<"{\"checks\":"<<checks<<"}\n";}
