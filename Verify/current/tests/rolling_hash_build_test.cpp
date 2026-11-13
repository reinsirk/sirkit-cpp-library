// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_RH_SOURCE
#define SIRKIT_RH_SOURCE "../../../String/RollingHash.cpp"
#endif
#include SIRKIT_RH_SOURCE
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
namespace {
using Big=boost::multiprecision::cpp_int;
constexpr auto p=RollingHash::modulus;
std::size_t checks=0,cases=0;
void require(bool v){++checks;if(!v)std::abort();}
std::uint64_t residue(const Big&v){return (v%p).convert_to<std::uint64_t>();}
void inspect(const RollingHash&h,const std::string&s,const std::vector<std::uint64_t>&prefix,const std::vector<std::uint64_t>&powers){
 require(h.size()==s.size());require(h.whole()==prefix.back());
 for(std::size_t i=0;i<=s.size();++i){require(h.get(0,i)==prefix[i]);if(i%31==0||i+17>=s.size()){const auto l=i/3;const auto expected=residue(Big(prefix[i])+Big(p)*p-Big(prefix[l])*powers[i-l]);require(h.get(l,i)==expected);require(h.concat(prefix[l],expected,i-l)==prefix[i]);}}
 if(!s.empty())require(h.get(s.size()-1,s.size())==static_cast<unsigned char>(s.back()));
}
}
int main(){
 const std::array<std::size_t,43> lengths{0,1,2,3,4,5,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129,255,256,257,4095,4096,4097,16383,16384,16385,16386,16387,16388,16389,32766,32767,32768,32769,32770,32771,131071,131072,131073};
 for(auto b:{std::uint64_t{0},std::uint64_t{2},std::uint64_t{256},std::uint64_t{1000000007},p/2,p-2,p-1}){
  const auto base=b<2?10:b>p-2?p-10:b;RollingHash reusable("x",b);
  for(auto n:lengths){++cases;std::string s(n,'\0');for(std::size_t i=0;i<n;++i)s[i]=static_cast<char>((i*137+(i>>7)*19+n)%256);
   std::vector<std::uint64_t> prefix(n+1),powers(n+1,1);for(std::size_t i=0;i<n;++i){prefix[i+1]=residue(Big(prefix[i])*base+static_cast<unsigned char>(s[i]));powers[i+1]=residue(Big(powers[i])*base);}
   RollingHash fresh(s,b);inspect(fresh,s,prefix,powers);reusable.assign(s);inspect(reusable,s,prefix,powers);
   reusable.assign(std::string_view(s).substr(0,n/2));reusable.assign(s);inspect(reusable,s,prefix,powers);
   std::string changed=s;
   for(auto&c:changed)c=static_cast<char>(static_cast<unsigned char>(c)^0xA5u);
   std::vector<std::uint64_t> changed_prefix(n+1);
   for(std::size_t i=0;i<n;++i)changed_prefix[i+1]=residue(Big(changed_prefix[i])*base+static_cast<unsigned char>(changed[i]));
   reusable.assign(changed);inspect(reusable,changed,changed_prefix,powers);
   reusable.assign(s);inspect(reusable,s,prefix,powers);
   RollingHash copied(fresh);fresh.clear();fresh.assign(s);inspect(fresh,s,prefix,powers);inspect(copied,s,prefix,powers);RollingHash moved(std::move(copied));require(copied.empty());inspect(moved,s,prefix,powers);
  }
 }
 // 底2で最初の61文字を1にするとhashがp=0(mod p)、以後NULで0を保つ。
 // 62文字目を処理した次block状態は冗長表現pになる。
 for(auto base:{std::uint64_t{2},p-2})for(auto n:{std::size_t{16384},std::size_t{16385},std::size_t{16386},std::size_t{16387},std::size_t{32769}})for(int pattern=0;pattern<3;++pattern){
  ++cases;std::string s(n,pattern==1?char(255):char(0));if(pattern==2)for(std::size_t i=0;i<61;++i)s[i]=char(1);
  std::vector<std::uint64_t> prefix(n+1),powers(n+1,1);for(std::size_t i=0;i<n;++i){prefix[i+1]=residue(Big(prefix[i])*base+static_cast<unsigned char>(s[i]));powers[i+1]=residue(Big(powers[i])*base);}
  RollingHash h(s,base);inspect(h,s,prefix,powers);if(base==2&&pattern==2){require(h.get(0,60)==(std::uint64_t{1}<<60)-1);require(h.get(0,61)==0);require(h.get(0,62)==0);require(h.whole()==0);}
  h.clear();h.assign(s);inspect(h,s,prefix,powers);
 }
 std::printf("PASS block boundary checks=%zu cases=%zu\n",checks,cases);
}
