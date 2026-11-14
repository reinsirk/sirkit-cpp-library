// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_UNRELEASED_VECTOR_SOURCE
#define SIRKIT_UNRELEASED_VECTOR_SOURCE "../../../Fast/Source/Internal/Unreleased_Vector.cpp"
#endif
#include SIRKIT_UNRELEASED_VECTOR_SOURCE
#include SIRKIT_UNRELEASED_VECTOR_SOURCE
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>
template<class T>using V=::sirkit::detail::UnreleasedVector<T>;
template<class T>T make(std::uint64_t n){if constexpr(std::is_same_v<T,std::string>)return std::string(50,static_cast<char>('a'+n%26))+std::to_string(n);else return static_cast<T>(n);}
template<class T>void values(std::mt19937_64&rng){V<T>v;std::vector<T>a;for(int q=0;q<10000;++q){T x=make<T>(rng()%1000);switch(rng()%14){case 0:v.push_back(x);a.push_back(x);break;case 1:v.emplace_back(x);a.emplace_back(x);break;case 2:if(!a.empty()){v.pop_back();a.pop_back();}break;case 3:{auto n=rng()%100;v.resize(n,x);a.resize(n,x);break;}case 4:{auto n=rng()%100;v.resize(n);a.resize(n);break;}case 5:{auto n=rng()%150;v.reserve(n);a.reserve(n);break;}case 6:if(!a.empty()){auto n=rng()%a.size();v.erase(v.begin()+n);a.erase(a.begin()+n);}break;case 7:{auto n=rng()%(a.size()+1);v.insert(v.begin()+n,x);a.insert(a.begin()+n,x);break;}case 8:if(!a.empty()){auto n=rng()%(a.size()+1);v.insert(v.begin()+n,v.back());a.insert(a.begin()+n,a.back());}break;case 9:{auto copy=v;assert(std::equal(copy.begin(),copy.end(),a.begin(),a.end()));auto moved=std::move(copy);assert(copy.empty());copy=std::move(moved);assert(moved.empty());copy=std::move(copy);copy=copy;assert(copy.size()==a.size());copy.clear();break;}case 10:v.clear();a.clear();break;case 11:{auto n=rng()%70;v.assign(n,x);a.assign(n,x);break;}case 12:if(!a.empty()){std::size_t l=rng()%(a.size()+1),r=rng()%(a.size()+1);if(l>r)std::swap(l,r);v.erase(v.begin()+l,v.begin()+r);a.erase(a.begin()+l,a.begin()+r);}break;case 13:v.shrink_to_fit();a.shrink_to_fit();break;}assert(v.size()==a.size()&&std::equal(v.begin(),v.end(),a.begin(),a.end()));if(!a.empty())assert(v.front()==a.front()&&v.back()==a.back());}v.clear();}
struct alignas(128)Aligned{std::uint64_t x=0;std::uint64_t padding[15]{};explicit Aligned(std::uint64_t n=0):x(n){}};
int main(){static_assert(sizeof(V<std::int64_t>)==sizeof(std::vector<std::int64_t>));static_assert(std::is_nothrow_move_constructible_v<V<std::unique_ptr<int>>>);static_assert(!std::is_copy_constructible_v<V<std::unique_ptr<int>>>);std::mt19937_64 rng(73319);values<int>(rng);values<std::string>(rng);values<bool>(rng);
 V<Aligned>aligned;for(int i=0;i<1000;++i)aligned.emplace_back(i);assert(reinterpret_cast<std::uintptr_t>(aligned.data())%alignof(Aligned)==0);for(int i=0;i<1000;++i)assert(aligned[i].x==static_cast<std::uint64_t>(i));auto ac=aligned;assert(ac[999].x==999);ac.clear();aligned.clear();
 V<std::unique_ptr<int>>owned;for(int i=0;i<1000;++i)owned.push_back(std::make_unique<int>(i));owned.erase(owned.begin()+3,owned.begin()+700);assert(*owned[3]==700);auto moved=std::move(owned);assert(owned.empty()&&*moved.back()==999);owned=std::move(moved);assert(moved.empty());owned=std::move(owned);assert(owned.size()==303);owned.clear();
 std::istringstream in("1 2 3 4");V<int>stream{std::istream_iterator<int>(in),std::istream_iterator<int>()};assert(stream.size()==4&&stream.back()==4);V<int>init{1,2,3};init.assign({4,5});init.insert(init.begin()+1,{6,7});assert(init[0]==4&&init[1]==6&&init[2]==7&&init[3]==5);V<int>from(std::vector<int>{8,9});assert(from.size()==2);swap(init,from);assert(init[0]==8);std::cout<<"unreleased helper random30000/overaligned/moveonly/selfmove/alias/inputiterator OK\n";
}
