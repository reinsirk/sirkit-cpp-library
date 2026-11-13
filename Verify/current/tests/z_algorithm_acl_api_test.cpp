// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Z_Algorithm.cpp"
#include "../../../String/Z_Algorithm.cpp"
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
namespace {
using Result=std::vector<int>;
using StringFn=Result(*)(const std::string&);
using VectorFn=Result(*)(const std::vector<int>&);
static_assert(std::is_same_v<decltype(static_cast<StringFn>(&z_algorithm)),StringFn>);
static_assert(std::is_same_v<decltype(static_cast<VectorFn>(&z_algorithm<int>)),VectorFn>);
std::size_t checks=0;
void check(bool value){++checks;if(!value)std::abort();}
template<class S>Result reference(const S& s){
    Result result(s.size());
    for(std::size_t i=0;i<s.size();++i)
        while(static_cast<std::size_t>(result[i])<s.size()-i&&s[result[i]]==s[i+result[i]])++result[i];
    return result;
}
template<class S>void compare(const S& s){check(z_algorithm(s)==reference(s));}
struct Huge {
    std::size_t length;
    std::size_t size()const{return length;}
    int operator[](int)const{std::abort();}
};
struct BooleanProxy {
    bool value;
    explicit operator bool()const{return value;}
    bool operator!()const=delete;
};
struct ProxyEquality {
    int value;
    BooleanProxy operator==(const ProxyEquality& other)const{return {value==other.value};}
};
struct EqualityOnly {
    int value;
    bool operator==(const EqualityOnly&)const=default;
};
}
int main(){
    const StringFn string_function=&z_algorithm;
    const VectorFn vector_function=&z_algorithm<int>;
    check(string_function("ababa")==Result({5,0,3,0,1}));
    check(vector_function({1,2,1,2})==Result({4,0,2,0}));
    check(z_algorithm("ababa")==Result({5,0,3,0,1}));
    check(z_algorithm<int>({1,2,1})==Result({3,0,1}));
    compare(std::string{});compare(std::vector<int>{});compare(std::array<int,0>{});
    for(int n=0;n<=10;++n)for(unsigned mask=0;mask<(1U<<n);++mask){
        std::string s(n,'a');std::vector<int> values(n);
        for(int i=0;i<n;++i){values[i]=static_cast<int>(mask>>i&1U)-1;s[i]=static_cast<char>('a'+(mask>>i&1U));}
        const auto saved=s;compare(s);compare(std::string_view(s));compare(values);compare(std::span<const int>(values));check(s==saved);
    }
    const std::string bytes{'\0','\xff','\0','\xff','\0'};compare(bytes);compare(std::string_view(bytes));
    compare(std::vector<bool>{true,false,true,false,true});
    compare(std::vector<std::int64_t>{INT64_MIN,INT64_MAX,INT64_MIN,INT64_MAX});
    compare(std::vector<std::uint64_t>{UINT64_MAX,0,UINT64_MAX});
    compare(std::array<EqualityOnly,4>{{{1},{2},{1},{2}}});
    compare(std::array<ProxyEquality,5>{{{1},{2},{1},{2},{1}}});
    check(z_algorithm<std::string>({"a","b","a"})==Result({3,0,1}));
    check(PrefixMatches(std::string_view("aba"),std::string_view("ababa"))==Result({3,0,3,0,1}));
    check(ZMatches(std::string_view("aba"),std::string_view("ababa"))==Result({0,2}));
    check(ZMatches(std::string_view{},std::string_view("ab"))==Result({0,1,2}));
    check(ZMatches(std::string_view{},std::string_view{})==Result({0}));
    check(ZMatches(std::string_view("x"),std::string_view{})==Result{});
    for(std::size_t n:{static_cast<std::size_t>(std::numeric_limits<int>::max()),std::numeric_limits<std::size_t>::max()}){
        bool rejected=false;try{(void)z_algorithm(Huge{n});}catch(const std::length_error&){rejected=true;}check(rejected);
    }
    std::printf("z_algorithm ACL API checks=%zu\n",checks);
}
