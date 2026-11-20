// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_SPARSE_TEST_SOURCE
#define SIRKIT_SPARSE_TEST_SOURCE "../../../DataStructure/Sparse_Table.cpp"
#endif
#include SIRKIT_SPARSE_TEST_SOURCE
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>
#include <algorithm>
std::uint64_t checks=0;
void check(bool x){++checks;if(!x)std::abort();}
template<class T>struct Minimum {T operator()(const T& a,const T& b)const{return std::min(a,b);}};
template<class T>void min_case(std::size_t n){
    std::mt19937_64 random(0x7195+n+sizeof(T));std::vector<T>a(n);
    for(auto&x:a){if constexpr(std::is_signed_v<T>)x=static_cast<T>(int(random()%101)-50);else x=static_cast<T>(random()%101);}
    const T identity=std::numeric_limits<T>::max();SparseTable<T,Minimum<T>>t(a,identity);
    for(int rep=0;rep<700;++rep){auto l=random()%(n+1),r=random()%(n+1);if(l>r)std::swap(l,r);T expected=identity;for(auto i=l;i<r;++i)expected=std::min(expected,a[i]);check(t.prod(l,r)==expected);}
    check(t.values()==a);auto copied=t;t.clear();check(copied.all_prod()==(n?*std::min_element(a.begin(),a.end()):identity));
}
struct Stats {std::uint64_t calls=0,hash=0x82149321;void add(std::int64_t a,std::int64_t b){++calls;hash=(hash^std::uint64_t(a))*0x100000001b3ULL;hash=(hash^std::uint64_t(b))*0x100000001b3ULL;}};
template<class T> struct First {
    Stats*stats;
    T operator()(const T&a,const T&b)const{stats->add(a,b);return a?a:b;}
};
template<class T> void first_case(std::size_t n){
    std::mt19937_64 random(0x3697+n);std::vector<T>a(n);for(auto&v:a)v=random()%4?0:T(random()%901+1);
    std::vector<T> previous=a;Stats expected;
    for(std::size_t width=2;width<=n;){const auto half=width/2,count=n-width+1;std::vector<T>next;next.reserve(count);for(std::size_t i=0;i<count;++i){expected.add(previous[i],previous[i+half]);next.push_back(previous[i]?previous[i]:previous[i+half]);}previous=std::move(next);if(width>n/2)break;width*=2;}
    Stats got;SparseTable<T,First<T>>t(a,0,First<T>{&got});check(got.calls==expected.calls);check(got.hash==expected.hash);
    for(int rep=0;rep<1200;++rep){auto l=random()%(n+1),r=random()%(n+1);if(l>r)std::swap(l,r);T result=0;for(auto i=l;i<r;++i)if(!result)result=a[i];check(t.prod(l,r)==result);}
}
struct Proxy {int value;std::uint64_t*conversions;operator int()const{++*conversions;return value;}};
struct MixedMinimum {
    std::uint64_t*conversions;
    Proxy operator()(const int&a,const int&b){return {std::min(a,b),conversions};}
    int operator()(const int&a,const int&b)const{return std::min(a,b);}
};
void proxy_case(){
    std::vector<int>a(1001);for(std::size_t i=0;i<a.size();++i)a[i]=int((i*97)%503);std::uint64_t expected=0;
    for(std::size_t width=2;width<=a.size();){expected+=a.size()-width+1;if(width>a.size()/2)break;width*=2;}
    std::uint64_t conversions=0;SparseTable<int,MixedMinimum>t(a,std::numeric_limits<int>::max(),MixedMinimum{&conversions});check(conversions==expected);
    for(std::size_t l=0;l<a.size();l+=7)for(std::size_t r=l+1;r<=a.size();r+=13)check(t.prod(l,r)==*std::min_element(a.begin()+l,a.begin()+r));
}
int main(){
    for(std::size_t n:{0,1,254,255,256,257,258,511,512,513,514,999,1000,1001,4097}){
        min_case<std::int8_t>(n);min_case<std::uint8_t>(n);min_case<std::int16_t>(n);min_case<std::uint16_t>(n);min_case<std::int32_t>(n);min_case<std::uint32_t>(n);min_case<std::int64_t>(n);min_case<std::uint64_t>(n);first_case<std::int32_t>(n);first_case<std::int64_t>(n);
    }
    proxy_case();std::printf("AVX/type/order checks=%ju\n",std::uintmax_t(checks));
}
