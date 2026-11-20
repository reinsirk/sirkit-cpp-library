// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef Z_HEADER
#define Z_HEADER "../../../String/Z_Algorithm.cpp"
#endif
#ifndef MANACHER_HEADER
#define MANACHER_HEADER "../../../String/Manacher.cpp"
#endif
#ifndef SUFFIX_HEADER
#define SUFFIX_HEADER "../../../String/Suffix_Array.cpp"
#endif
#include Z_HEADER
#include Z_HEADER
#include MANACHER_HEADER
#include MANACHER_HEADER
#include SUFFIX_HEADER
#include SUFFIX_HEADER
#include <cassert>
#include <cstdio>
#include <numeric>
#include <random>
#include <string>
#include <tuple>
#include <type_traits>

namespace {
std::uint64_t checks = 0;
void check(bool value) { ++checks; if (!value) std::abort(); }
template<class Exception, class Function> void rejects(Function function) {
    bool rejected = false; try { function(); } catch (const Exception&) { rejected = true; }
    check(rejected);
}
template<class Sequence> auto element(const Sequence& a, int i) {
    using T = typename Sequence::value_type;
    if constexpr (std::is_same_v<T,char>) return static_cast<unsigned char>(a[i]);
    else return static_cast<T>(a[i]);
}
template<class Sequence> std::vector<int> naive_sa(const Sequence& a) {
    const int n=static_cast<int>(a.size());std::vector<int> sa(n);std::iota(sa.begin(),sa.end(),0);
    std::sort(sa.begin(),sa.end(),[&](int x,int y){while(x<n&&y<n){if(element(a,x)!=element(a,y))return element(a,x)<element(a,y);++x;++y;}return x==n&&y!=n;});return sa;
}
template<class Sequence> void test_sequence(const Sequence& a) {
    const int n=static_cast<int>(a.size());const auto saved=a;const auto z=z_algorithm(a);
    for(int i=0;i<n;++i){int length=0;while(length<n-i&&a[length]==a[i+length])++length;check(z[i]==length);}
    const auto sa=suffix_array(a);check(sa==naive_sa(a));const auto lcp=lcp_array(a,sa);
    check(lcp.size()==(n?static_cast<std::size_t>(n-1):0));
    for(int i=0;i+1<n;++i){int length=0;while(length<n-sa[i]&&length<n-sa[i+1]&&a[sa[i]+length]==a[sa[i+1]+length])++length;check(lcp[i]==length);}
    const PalindromeRadii pal(a);std::uint64_t count=0;std::pair<int,int> longest{0,0};
    for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){bool yes=true;for(int x=l,y=r-1;x<y;++x,--y)if(a[x]!=a[y]){yes=false;break;}check(pal.is_palindrome(l,r)==yes);if(yes&&l!=r){++count;if(r-l>longest.second-longest.first)longest={l,r};}}
    check(pal.count()==count);check(pal.longest()==longest);const auto lengths=PalindromicLengths(a);
    check(lengths.size()==(n?2*static_cast<std::size_t>(n)-1:0));
    for(int i=0;i<n;++i){int radius=1;while(radius<=i&&radius<n-i&&a[i-radius]==a[i+radius])++radius;check(pal.odd[i]==radius);check(lengths[2*static_cast<std::size_t>(i)]==2*radius-1);radius=0;while(radius<i&&radius<n-i&&a[i-radius-1]==a[i+radius])++radius;check(pal.even[i]==radius);if(i>0)check(lengths[2*static_cast<std::size_t>(i)-1]==2*radius);}
    check(a==saved);
}
template<class Sequence> void test_matching(const Sequence& pattern,const Sequence& text) {
    const auto lengths=PrefixMatches(pattern,text);std::vector<int> expected;
    for(int i=0;i<static_cast<int>(text.size());++i){int length=0;while(length<static_cast<int>(pattern.size())&&length<static_cast<int>(text.size())-i&&pattern[length]==text[i+length])++length;check(lengths[i]==length);if(length==static_cast<int>(pattern.size()))expected.push_back(i);}
    if(pattern.empty())expected.push_back(static_cast<int>(text.size()));
    check(ZMatches(pattern,text)==expected);
    const auto sa=suffix_array(text);const auto bounds=SuffixArrayEqualRange(text,sa,pattern);
    int low=0,high=0;
    for(int position:sa){std::size_t j=0;int cmp=0;while(j<pattern.size()&&j<text.size()-position){const auto x=element(text,position+static_cast<int>(j)),y=element(pattern,static_cast<int>(j));if(x!=y){cmp=x<y?-1:1;break;}++j;}if(cmp==0&&j<pattern.size())cmp=-1;if(cmp<0)++low;if(cmp<=0)++high;}
    check(bounds==std::pair{low,high});
}
struct FakeLength {std::size_t n;std::size_t size()const{return n;}int operator[](int)const{std::abort();}};
struct ThrowingText {std::size_t size()const{return 20;}int operator[](int)const{throw std::runtime_error("compare");}};
}
int main(){
    std::mt19937_64 rng(0x1d5a7a1120261031ULL);
    for(int n=0;n<=11;++n)for(unsigned mask=0;mask<(1u<<n);++mask){std::string s(n,'a');for(int i=0;i<n;++i)s[i]=static_cast<char>('a'+((mask>>i)&1));test_sequence(s);}
    for(int rep=0;rep<700;++rep){const int n=static_cast<int>(rng()%180);std::string s(n,' ');for(auto&c:s)c=static_cast<char>(rng()%256);test_sequence(s);std::string pattern(static_cast<std::size_t>(rng()%55),' ');for(auto&c:pattern)c=static_cast<char>(rng()%256);test_matching(pattern,s);}
    for(int rep=0;rep<700;++rep){std::vector<std::int64_t>a(static_cast<std::size_t>(rng()%55));for(auto&x:a)x=static_cast<std::int64_t>(rng()%13)-6;test_sequence(a);std::vector<std::int64_t>b(static_cast<std::size_t>(rng()%12));for(auto&x:b)x=static_cast<std::int64_t>(rng()%13)-6;test_matching(b,a);std::vector<int>bounded(a.size());for(std::size_t i=0;i<a.size();++i)bounded[i]=static_cast<int>(a[i]+6);check(suffix_array(bounded,12)==naive_sa(bounded));}
    test_sequence(std::vector<std::int64_t>{INT64_MIN,INT64_MAX,0,-1,INT64_MIN});test_sequence(std::vector<std::uint64_t>{UINT64_MAX,0,UINT64_MAX-1,0});test_sequence(std::vector<bool>{true,false,true,true,false});
    for(int n:{0,1,2,3,16,63,256,1025}){std::string all(n,'x'),alternate(n,'a'),bytes(n,' ');for(int i=0;i<n;++i){alternate[i]=static_cast<char>('a'+i%2);bytes[i]=static_cast<char>(i%256);}for(const auto*s:{&all,&alternate,&bytes}){check(suffix_array(*s)==naive_sa(*s));test_matching(s->substr(0,std::min(n,30)),*s);}}
    const int large_n=200000;std::string all(large_n,'x');const auto sa=suffix_array(all),lcp=lcp_array(all,sa),z=z_algorithm(all);for(int i=0;i<large_n;++i){check(sa[i]==large_n-1-i);check(z[i]==large_n-i);if(i+1<large_n)check(lcp[i]==i+1);}const PalindromeRadii pal(all);check(pal.count()==std::uint64_t{large_n}*(large_n+1)/2);check(pal.longest()==std::pair{0,large_n});
    rejects<std::length_error>([]{z_algorithm(FakeLength{static_cast<std::size_t>(INT32_MAX)});});rejects<std::length_error>([]{PalindromeRadii p(FakeLength{static_cast<std::size_t>(INT32_MAX)});});rejects<std::length_error>([]{lcp_array(FakeLength{static_cast<std::size_t>(INT32_MAX)},{});});rejects<std::length_error>([]{suffix_array(std::vector<int>{},-1);});
    PalindromeRadii keep(std::string("abba"));const auto before_odd=keep.odd,before_even=keep.even;rejects<std::runtime_error>([&]{keep.assign(ThrowingText{});});check(keep.odd==before_odd&&keep.even==before_even);
#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::out_of_range>([&]{(void)keep.is_palindrome(-1,2);});rejects<std::out_of_range>([&]{(void)keep.is_palindrome(2,1);});rejects<std::out_of_range>([&]{(void)keep.is_palindrome(0,5);});rejects<std::out_of_range>([]{suffix_array(std::vector<int>{-1},3);});rejects<std::out_of_range>([]{suffix_array(std::vector<int>{4},3);});rejects<std::invalid_argument>([]{lcp_array(std::string("a"),{});});rejects<std::invalid_argument>([]{lcp_array(std::string("aa"),{0,0});});rejects<std::invalid_argument>([]{lcp_array(std::string("a"),{1});});rejects<std::out_of_range>([]{SuffixArrayEqualRange(std::string_view("a"),{-1},std::string_view("a"));});
#endif
    std::printf("string reference checks=%ju\n",static_cast<std::uintmax_t>(checks));
}
