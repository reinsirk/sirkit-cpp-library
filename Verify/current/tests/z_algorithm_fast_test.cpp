// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
// 大きなZ配列と検索補助を、接尾辞配列/LCPの独立参照と照合する。
#ifndef Z_HEADER
#define Z_HEADER "../../../String/Z_Algorithm.cpp"
#endif
#include Z_HEADER
#include "../../../String/Suffix_Array.cpp"
#include <algorithm>
#include <array>
#include <climits>
#include <cstdint>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <random>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>
namespace {
using Z=std::vector<int>;
std::uint64_t checks=0,cases=0;
void check(bool ok,const std::source_location where=std::source_location::current()){
    ++checks;
    if(!ok){std::fprintf(stderr,"z fast reference failed %s:%u\n",where.file_name(),where.line());std::abort();}
}
template<class S>Z naive(const S& s){
    const int n=static_cast<int>(s.size());Z z(n);
    for(int i=0;i<n;++i)while(z[i]<n-i&&s[z[i]]==s[i+z[i]])++z[i];
    return z;
}
template<class S>Z from_suffix_array(const S& s){
    const int n=static_cast<int>(s.size());if(!n)return {};
    const auto sa=suffix_array(s),lcp=lcp_array(s,sa);Z answer(n);answer[0]=n;
    int rank=0;while(sa[rank]!=0)++rank;
    int common=n;
    for(int j=rank-1;j>=0;--j){common=std::min(common,lcp[j]);answer[sa[j]]=common;}
    common=n;
    for(int j=rank+1;j<n;++j){common=std::min(common,lcp[j-1]);answer[sa[j]]=common;}
    return answer;
}
std::string make_text(int n,int shape,std::mt19937_64& rng){
    std::string s(n,'a');
    if(shape==1)for(int i=0;i<n;++i)s[i]=static_cast<char>('a'+i%3);
    if(shape==2&&n)s[0]='b';
    if(shape==3&&n)s[n-1]='b';
    if(shape==4&&n)s[n/2]='b';
    if(shape==5)for(auto& c:s)c=static_cast<char>('a'+rng()%2);
    if(shape==6)for(auto& c:s)c=static_cast<char>(rng()%256);
    return s;
}
void string_case(const std::string& s){
    ++cases;const auto saved=s;const auto expected=from_suffix_array(std::string_view(s));
    if(s.size()<=128)check(expected==naive(s));
    check(z_algorithm(s)==expected);check(z_algorithm(std::string_view(s))==expected);
    check(s==saved);
}
void integer_cases(std::mt19937_64& rng){
    for(int n:{31,128,4095,4097,8193}){
        std::vector<std::int64_t> a(n);std::vector<bool> bits(n);
        for(int i=0;i<n;++i){a[i]=static_cast<std::int64_t>(rng()%17)-8;bits[i]=(rng()&1)!=0;}
        if(n>2){a[0]=std::numeric_limits<std::int64_t>::min();a[n/2]=a[0];a[n-1]=std::numeric_limits<std::int64_t>::max();}
        for(int kind=0;kind<2;++kind){++cases;
            const auto expected=kind==0?from_suffix_array(a):from_suffix_array(bits);
            if(n<=128)check(expected==(kind==0?naive(a):naive(bits)));
            check(expected==(kind==0?z_algorithm(a):z_algorithm(bits)));
        }
    }
}
Z prefix_reference(const std::string& p,const std::string& t){
    if(p.empty())return Z(t.size());
    std::vector<int> joined;joined.reserve(p.size()+t.size()+1);
    for(unsigned char c:p)joined.push_back(c);
    joined.push_back(256);
    for(unsigned char c:t)joined.push_back(c);
    const auto complete=from_suffix_array(joined);Z result(t.size());
    for(std::size_t i=0;i<t.size();++i)result[i]=std::min(static_cast<int>(p.size()),complete[p.size()+1+i]);
    return result;
}
void matching_case(const std::string& p,const std::string& t){
    const auto before_p=p,before_t=t;const auto expected=prefix_reference(p,t);Z positions;
    if(p.size()<=128&&t.size()<=128){Z slow(t.size());
        for(std::size_t i=0;i<t.size();++i)while(static_cast<std::size_t>(slow[i])<p.size()&&static_cast<std::size_t>(slow[i])<t.size()-i&&p[slow[i]]==t[i+slow[i]])++slow[i];
        check(slow==expected);
    }
    for(int i=0;i<static_cast<int>(t.size());++i)if(expected[i]==static_cast<int>(p.size()))positions.push_back(i);
    if(p.empty())positions.push_back(static_cast<int>(t.size()));
    check(PrefixMatches(p,t)==expected);check(PrefixMatches(std::string_view(p),std::string_view(t))==expected);
    check(ZMatches(p,t)==positions);check(ZMatches(std::string_view(p),std::string_view(t))==positions);
    check(p==before_p&&t==before_t);++cases;
}
void matches(std::mt19937_64& rng){
    matching_case({},{});matching_case({},make_text(127,6,rng));matching_case(make_text(4096,0,rng),{});
    for(int n:{31,128})matching_case(make_text(n/2,1,rng),make_text(n,1,rng));
    matching_case(make_text(4095,0,rng),make_text(8193,0,rng));
    matching_case(make_text(4096,0,rng),make_text(12300,4,rng));
    matching_case(make_text(4097,1,rng),make_text(8193,1,rng));
    auto p=make_text(4128,6,rng),t=make_text(22000,6,rng);
    t.replace(37,p.size(),p);t.replace(5000,p.size(),p);matching_case(p,t);
}

}
int main(){
    std::mt19937_64 rng(0x416042112026ULL);
    for(int n:{0,1,2,31,32,33,63,64,65,127,128})for(int shape:{0,1,6})string_case(make_text(n,shape,rng));
    for(int n:{4095,4096,4097,4127,4128,4129,8191,8192,8193})for(int shape=0;shape<7;++shape)string_case(make_text(n,shape,rng));
    integer_cases(rng);matches(rng);
    std::printf("z fast reference cases=%" PRIu64 " checks=%" PRIu64 "\n",cases,checks);
}
