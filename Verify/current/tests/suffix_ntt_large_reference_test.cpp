// SPDX-License-Identifier: CC0-1.0
#ifndef SUFFIX_HEADER
#define SUFFIX_HEADER "../../../String/Suffix_Array.cpp"
#endif
#ifndef NTT_HEADER
#define NTT_HEADER "../../../Math/NTT.cpp"
#endif
#include SUFFIX_HEADER
#include NTT_HEADER
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <string>
namespace {
std::uint64_t checks=0;void check(bool x){++checks;if(!x)std::abort();}
std::vector<int> doubling(const std::string&s){const int n=static_cast<int>(s.size());std::vector<int>sa(n),rank(n),next(n);std::iota(sa.begin(),sa.end(),0);for(int i=0;i<n;++i)rank[i]=static_cast<unsigned char>(s[i]);for(int step=1;step<n;step*=2){auto key=[&](int i){return std::pair{rank[i],step<n-i?rank[i+step]:-1};};std::sort(sa.begin(),sa.end(),[&](int a,int b){return key(a)<key(b);});next[sa[0]]=0;for(int i=1;i<n;++i)next[sa[i]]=next[sa[i-1]]+(key(sa[i-1])!=key(sa[i]));rank.swap(next);if(rank[sa.back()]==n-1)break;if(step>n/2)break;}return sa;}
}
int main(){std::mt19937_64 rng(0x514241cc031ULL);for(int mode=0;mode<4;++mode){std::string s(200000,'a');for(int i=0;i<200000;++i){if(mode==1)s[i]=static_cast<char>('a'+i%2);if(mode==2)s[i]=static_cast<char>(rng()%256);if(mode==3)s[i]="mississippi"[i%11];}check(suffix_array(s)==doubling(s));}
std::string a="a",b="b";while(b.size()<200000){auto next=a+b;a=std::move(b);b=std::move(next);}b.resize(200000);check(suffix_array(b)==doubling(b));
NTT<>engine;const std::size_t n=262177,m=262143;std::vector<std::uint32_t>x(n,1),y(m,998244352);const auto answer=engine.convolution_normalized(std::move(x),std::move(y));check(answer.size()==n+m-1);for(std::size_t i=0;i<answer.size();++i){const auto count=static_cast<std::uint32_t>(std::min({i+1,n,m,n+m-1-i}));check(answer[i]==998244353-count);}std::vector<std::uint32_t>data(std::size_t{1}<<20);for(auto&v:data)v=static_cast<std::uint32_t>(rng()%998244353);const auto saved=data;engine.transform(data);engine.transform(data,true);check(data==saved);std::printf("large reference checks=%ju\n",static_cast<std::uintmax_t>(checks));}
