// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_RH_SOURCE
#define SIRKIT_RH_SOURCE "../../../String/RollingHash.cpp"
#endif
#include SIRKIT_RH_SOURCE
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string>
int main(){
 std::uint64_t checks=0;
 for(auto base:std::array<std::uint64_t,8>{0,2,10,1000000007,RollingHash::modulus/2,RollingHash::modulus-2,RollingHash::modulus-1,std::numeric_limits<std::uint64_t>::max()}){
  std::string s,t="different context";std::array<std::size_t,256>ls{},rs{};
  std::string common;for(int k=0;k<17;++k)common.push_back(static_cast<char>((k*113+7)&255));
  for(int i=0;i<256;++i){ls[i]=s.size();s+=common;s.push_back(static_cast<char>(i));s.push_back(static_cast<char>(i^91));rs[i]=t.size();t+=common;t.push_back(static_cast<char>(i));t.push_back(static_cast<char>(i^173));}
  RollingHash a(s,base),b(t,base);
  for(int i=0;i<256;++i)for(int j=0;j<256;++j){int expected=(i>j)-(i<j);
   if(a.compare(ls[i],ls[i]+18,b,rs[j],rs[j]+18)!=expected)throw std::runtime_error("long prefix byte sign");
   ++checks;
   if(a.compare(ls[i]+17,ls[i]+18,b,rs[j]+17,rs[j]+18)!=expected)throw std::runtime_error("first byte sign");
   ++checks;
   if(a.equal(ls[i],ls[i]+18,b,rs[j],rs[j]+18)!=(i==j))throw std::runtime_error("fused equal");
   ++checks;
  }
  if(!a.equal(0,0,b,t.size(),t.size()))throw std::runtime_error("empty equality");
  ++checks;
 }

 const auto req=[&](bool ok,const char*what){++checks;if(!ok)throw std::runtime_error(what);};
 // 開始prefixが一致する場合・末尾の空・ムーブ後の空を区別する。
 for(auto base:{std::uint64_t{2},std::uint64_t{911382323},RollingHash::modulus-2}){
  const std::string zeros(129,'\0');RollingHash a(zeros,base),b(zeros,base);
  for(auto l:{std::size_t{0},std::size_t{1},std::size_t{17},std::size_t{128}})for(auto ol:{std::size_t{0},std::size_t{1},std::size_t{17},std::size_t{128}}){auto n=std::min(zeros.size()-l,zeros.size()-ol);req(a.equal(l,l+n,b,ol,ol+n),"delta0 equal");req(a.compare(l,l+n,b,ol,ol+n)==0,"delta0 compare");}
  RollingHash moved(std::move(a));req(a.equal(0,0,b,129,129),"moved empty equality");req(a.compare(0,0,b,0,1)==-1,"empty less");req(!a.equal(0,0,b,0,1),"empty unequal length");b.clear();req(a.equal(0,0,b,0,0),"cleared equality");
 }
 // 開始prefix差と終端差の積和がちょうどp^2になる向きも照合する。
 {const auto p=RollingHash::modulus;std::string suffix(59,'\0');suffix+=std::string(2,char(1));const RollingHash tail(suffix,p-2),prefixed(std::string(2,char(1))+suffix,p-2);req(tail.whole()==p-1&&prefixed.whole()==0,"p squared preparation");req(prefixed.equal(2,63,tail,0,61),"p squared equal");req(tail.equal(0,61,prefixed,2,63),"reverse p squared equal");}
 // 意図的な衝突ではequalはハッシュ一致、compareは返ったLCP位置のbyte順を保つ。
 {const std::string one(61,char(1)),zero(61,char(0));const RollingHash a(one,2),b(zero,2);req(a.equal(0,61,b,0,61),"zero residue collision");
  for(const auto&pair:{std::pair<std::string,std::string>{one,zero},{std::string("ab")+one,std::string("c")+zero}}){const auto l=pair.first.size()-61,ol=pair.second.size()-61;const RollingHash h(pair.first,2),k(pair.second,2);req(h.equal(l,l+61,k,ol,ol+61),"nonzero context collision equal");const auto common=h.lcp(l,k,ol,61);int expected=0;if(common<61){auto x=static_cast<unsigned char>(pair.first[l+common]),y=static_cast<unsigned char>(pair.second[ol+common]);expected=(x>y)-(x<y);}req(h.compare(l,l+61,k,ol,ol+61)==expected,"collision compare byte at returned lcp");}
 }
 for(auto base:{std::uint64_t{2},std::uint64_t{911382323},RollingHash::modulus-2})for(unsigned bit:{4U,7U,8U,13U,14U,15U,17U})for(int delta:{-1,0,1})for(int context=0;context<3;++context){
  const std::size_t n=(std::size_t{1}<<bit)+delta;std::string common(n,'\0');for(std::size_t i=0;i<n;++i)common[i]=static_cast<char>((i*137+19)%256);const std::string pre=context==1?"a":"",other_pre=context==0?"":"b";const std::string s=pre+common+char(0)+char(44),t=other_pre+common+char(255)+char(55);const auto l=pre.size(),ol=other_pre.size();const RollingHash a(s,base),b(t,base);
  req(a.lcp(l,b,ol,n+1)==n,"long lcp boundary");req(a.lcp(l,b,ol,n)==n,"long lcp limited");req(a.compare(l,l+n+1,b,ol,ol+n+1)==-1,"long last mismatch");req(b.compare(ol,ol+n+1,a,l,l+n+1)==1,"long last mismatch reverse");req(a.equal(l,l+n,b,ol,ol+n),"long prefix equal");req(!a.equal(l,l+n+1,b,ol,ol+n+1),"long mismatch unequal");req(a.compare(l,l+n,b,ol,ol+n)==0,"long prefix compare");
 }
 std::printf("PASS %ju compare/equal checks; all unsigned-byte pairs, 17-byte prefix, 8 bases\n",static_cast<std::uintmax_t>(checks));
}
