// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
// 長い回文を、正順+区切り+逆順の接尾辞配列/LCPから独立に照合する。
#ifndef MANACHER_HEADER
#define MANACHER_HEADER "../../../String/Manacher.cpp"
#endif
#ifndef SUFFIX_HEADER
#define SUFFIX_HEADER "../../../String/Suffix_Array.cpp"
#endif
#include MANACHER_HEADER
#include MANACHER_HEADER
#include SUFFIX_HEADER
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
namespace {
std::uint64_t checks=0,cases=0;
void check(bool yes,const std::source_location where=std::source_location::current()) {
    ++checks;
    if(!yes){std::fprintf(stderr,"manacher independent failed %s:%u\n",where.file_name(),where.line());std::abort();}
}
struct LcpQueries {
    int base=1;
    std::vector<int> rank,tree;
    explicit LcpQueries(const std::vector<int>& data) {
        const auto sa=suffix_array(data,256),lcp=lcp_array(data,sa);
        rank.resize(data.size());
        for(int i=0;i<static_cast<int>(sa.size());++i)rank[sa[i]]=i;
        while(base<static_cast<int>(lcp.size()))base*=2;
        tree.assign(2*base,std::numeric_limits<int>::max());
        std::copy(lcp.begin(),lcp.end(),tree.begin()+base);
        for(int i=base-1;i>0;--i)tree[i]=std::min(tree[2*i],tree[2*i+1]);
    }
    int common(int a,int b)const {
        int left=rank[a],right=rank[b];
        if(left>right)std::swap(left,right);
        int answer=std::numeric_limits<int>::max();
        for(left+=base,right+=base;left<right;left/=2,right/=2){
            if(left&1)answer=std::min(answer,tree[left++]);
            if(right&1)answer=std::min(answer,tree[--right]);
        }
        return answer;
    }
};
struct Oracle {std::vector<int> odd,even,lengths;std::pair<int,int> longest{0,0};std::uint64_t count=0;};
Oracle reference(const std::vector<int>& data) {
    const int n=static_cast<int>(data.size());Oracle answer;
    answer.odd.resize(n);answer.even.resize(n);
    if(!n)return answer;
    std::vector<int> joined=data;joined.push_back(256);
    joined.insert(joined.end(),data.rbegin(),data.rend());
    const LcpQueries lcp(joined);
    const auto consider=[&](int left,int right){
        const int length=right-left,best=answer.longest.second-answer.longest.first;
        if(length>best||(length==best&&left<answer.longest.first))answer.longest={left,right};
    };
    answer.lengths.resize(2*static_cast<std::size_t>(n)-1);
    for(int i=0;i<n;++i){
        const int odd=std::min({i+1,n-i,lcp.common(i,2*n-i)});
        const int even=i?std::min({i,n-i,lcp.common(i,2*n+1-i)}):0;
        answer.odd[i]=odd;answer.even[i]=even;
        answer.lengths[2*static_cast<std::size_t>(i)]=2*odd-1;
        if(i)answer.lengths[2*static_cast<std::size_t>(i)-1]=2*even;
        answer.count+=static_cast<std::uint64_t>(odd)+static_cast<std::uint64_t>(even);
        consider(i-odd+1,i+odd);consider(i-even,i+even);
    }
    return answer;
}
template<class Sequence>void check_case(const Sequence& s,const std::vector<int>& codes) {
    check(s.size()==codes.size());const auto expected=reference(codes);const PalindromeRadii p(s);
    check(p.size()==static_cast<int>(s.size()));check(p.empty()==s.empty());
    check(p.odd==expected.odd);check(p.even==expected.even);
    check(p.count()==expected.count);check(p.longest()==expected.longest);
    check(PalindromicLengths(s)==expected.lengths);
    for(int i=0;i<p.size();++i){
        const int odd=expected.odd[i],even=expected.even[i];
        check(p.is_palindrome(i-odd+1,i+odd));
        if(odd<=i&&odd<p.size()-i)check(!p.is_palindrome(i-odd,i+odd+1));
        check(p.is_palindrome(i-even,i+even));
        if(even<i&&even<p.size()-i)check(!p.is_palindrome(i-even-1,i+even+1));
    }
    check(p.is_palindrome(0,0));check(p.is_palindrome(p.size(),p.size()));++cases;
}
void string_case(const std::string& s) {
    const auto saved=s;std::vector<int> codes;codes.reserve(s.size());
    for(unsigned char c:s)codes.push_back(c);
    check_case(s,codes);check(s==saved);
    if(s.size()<=8193)check_case(std::string_view(s),codes);
}
std::string make_text(int n,int shape,std::mt19937_64& rng) {
    std::string s(n,'a');
    if(shape==1)for(int i=0;i<n;++i)s[i]=static_cast<char>('a'+i%2);
    if(shape==2)for(int i=0;i<n;++i)s[i]=static_cast<char>('a'+i%3);
    if(shape==3)for(char& c:s)c=static_cast<char>(rng()%256);
    if(shape==4){
        for(int i=0;i<(n+1)/2;++i)s[i]=static_cast<char>(rng()%256);
        if(n>=512)for(int i=0;i<256;++i)s[i]=static_cast<char>(i);
        for(int i=0;i<n/2;++i)s[n-1-i]=s[i];
    }
    if(shape==5){s=make_text(n,4,rng);if(n>2)s[n/3]=static_cast<char>(static_cast<unsigned char>(s[n/3])^0x80U);}
    return s;
}
struct BooleanProxy {bool value;explicit operator bool()const{return value;}bool operator!()const=delete;};
struct OnlyEquality {
    int value;
    explicit OnlyEquality(int v):value(v){}
    OnlyEquality(const OnlyEquality&)=delete;
    OnlyEquality& operator=(const OnlyEquality&)=delete;
    BooleanProxy operator==(const OnlyEquality& other)const{return {value==other.value};}
};
struct EqualitySequence {
    std::array<OnlyEquality,9> data{{OnlyEquality(1),OnlyEquality(2),OnlyEquality(1),OnlyEquality(2),OnlyEquality(2),OnlyEquality(1),OnlyEquality(2),OnlyEquality(1),OnlyEquality(3)}};
    std::size_t size()const{return data.size();}
    bool empty()const{return data.empty();}
    const OnlyEquality& operator[](int i)const{return data[i];}
};
struct Huge {std::size_t n;std::size_t size()const{return n;}int operator[](int)const{std::abort();}};
struct ComparisonBomb {
    int value;int* budget;
    bool operator==(const ComparisonBomb& other)const {
        if(*budget==0)throw std::runtime_error("comparison budget");
        --*budget;return value==other.value;
    }
};
void generic_and_exceptions(std::mt19937_64& rng) {
    std::vector<std::int64_t> values(8193);std::vector<int> codes(values.size());std::vector<bool> bits(values.size());
    constexpr std::array<std::int64_t,4> symbols{INT64_MIN,INT64_MAX,-17,99};
    for(std::size_t i=0;i<values.size();++i){codes[i]=static_cast<int>(rng()%4);values[i]=symbols[codes[i]];bits[i]=(rng()&1U)!=0;}
    const auto saved_values=values;const auto saved_bits=bits;check_case(values,codes);check(values==saved_values);
    for(std::size_t i=0;i<bits.size();++i)codes[i]=bits[i]?1:0;
    check_case(bits,codes);check(bits==saved_bits);
    const EqualitySequence eq;check_case(eq,{1,2,1,2,2,1,2,1,3});
    EqualitySequence same_prefix;for(auto& v:same_prefix.data)v.value=7;same_prefix.data[3].value=9;
    check_case(same_prefix,{7,7,7,9,7,7,7,7,7});
    PalindromeRadii keep(std::string("abba"));const auto before_odd=keep.odd,before_even=keep.even;
    for(std::size_t n:{static_cast<std::size_t>(std::numeric_limits<int>::max()),std::numeric_limits<std::size_t>::max()}){
        bool rejected=false;try{keep.assign(Huge{n});}catch(const std::length_error&){rejected=true;}
        check(rejected);check(keep.odd==before_odd&&keep.even==before_even);
        rejected=false;try{(void)PalindromicLengths(Huge{n});}catch(const std::length_error&){rejected=true;}check(rejected);
    }
    int budget=std::numeric_limits<int>::max();std::vector<ComparisonBomb> text;
    for(int i=0;i<129;++i)text.push_back({i%3,&budget});
    const PalindromeRadii measured(text);const int comparisons=std::numeric_limits<int>::max()-budget;check(comparisons>1);
    for(int allowance:{0,1,comparisons/2,comparisons-1}){
        budget=allowance;bool rejected=false;
        try{keep.assign(text);}catch(const std::runtime_error&){rejected=true;}
        check(rejected);check(keep.odd==before_odd&&keep.even==before_even);
    }
    budget=std::numeric_limits<int>::max();keep.assign(text);check(keep.odd==measured.odd&&keep.even==measured.even);
    keep.assign(std::string{});check(keep.empty()&&keep.odd.empty()&&keep.even.empty()&&keep.count()==0&&keep.longest()==std::pair{0,0});
}
}
int main(){
    std::mt19937_64 rng(0x416358112026ULL);
    for(int n:{0,1,2,3,31,32,33,127,128})for(int shape:{0,1,3,4,5})string_case(make_text(n,shape,rng));
    for(int n:{4095,4096,4097,8193})for(int shape:{2,3,4,5})string_case(make_text(n,shape,rng));
    for(int shape:{0,1,4,5})string_case(make_text(500000,shape,rng));
    string_case(make_text(499999,4,rng));generic_and_exceptions(rng);
    std::printf("manacher independent cases=%ju checks=%ju\n",static_cast<std::uintmax_t>(cases),static_cast<std::uintmax_t>(checks));
}
