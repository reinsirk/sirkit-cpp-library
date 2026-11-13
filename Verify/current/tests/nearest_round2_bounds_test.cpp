// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_NEAREST_TEST_SOURCE
#define SIRKIT_NEAREST_TEST_SOURCE "../../../Geometry/Nearest_Two_points.cpp"
#endif
#include SIRKIT_NEAREST_TEST_SOURCE
#include <array>
#include <boost/multiprecision/cpp_int.hpp>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <bit>
using Big=boost::multiprecision::cpp_int;
std::uint64_t checks=0,pairs=0;
void require(bool b){++checks;if(!b)std::abort();}
struct D{std::uint64_t lo;bool hi;friend bool operator<(D a,D b){return a.hi!=b.hi?!a.hi:a.lo<b.lo;}};
D distance(P a,P b){std::uint64_t x=static_cast<std::uint64_t>(std::int64_t(a.first)-b.first),y=static_cast<std::uint64_t>(std::int64_t(a.second)-b.second);auto xx=x*x,yy=y*y;return{xx+yy,xx+yy<xx};}
void check(const std::vector<P>&p){D best{UINT64_MAX,true};for(std::size_t i=0;i<p.size();++i)for(auto j=i+1;j<p.size();++j){++pairs;auto d=distance(p[i],p[j]);if(d<best)best=d;}const auto original=p;auto inspect=[&](auto f){bool overflow=false;std::optional<NearestTwoPointsResult>r;try{r=f();}catch(const std::overflow_error&){overflow=true;}require(overflow==best.hi);if(!best.hi){require(r&&r->distance_squared==best.lo);require(r->first_index<r->second_index&&r->second_index<p.size());require(distance(p[r->first_index],p[r->second_index]).lo==best.lo);}require(p==original);};inspect([&]{return nearest_two_points(p);});inspect([&]{return nearest_two_points(p.begin(),static_cast<std::ptrdiff_t>(p.size()));});auto sorted=p;std::sort(sorted.begin(),sorted.end());auto r=nearest_two_points_sorted(sorted);require(bool(r));if(!best.hi)require(r->distance_squared==best.lo);}
int main(){std::mt19937 rng(994820);for(int k=0;k<10000;++k){P a{std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(rng())),std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(rng()))},b{std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(rng())),std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(rng()))};Big x=Big(a.first)-b.first,y=Big(a.second)-b.second,d=x*x+y*y;auto q=distance(a,b);require(q.hi==(d>UINT64_MAX));require(q.lo==(d&((Big{1}<<64)-1)).convert_to<std::uint64_t>());}
for(int n:{2047,2048,2049,4096})for(int shape=0;shape<6;++shape){std::vector<P>p;p.reserve(n);const int ybound=shape==0?92681:92682;for(int i=0;i<n;++i){std::int32_t x=std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(rng())),y=rng()%(ybound+1);if(shape==2){x=78;y=(i%17)*1024;}if(shape==3){x=i*7919;y=48;}if(shape==4){x=i*7819;y=i*7727;}if(shape==5){x=(i/64)*1000000+(i%8)*100;y=((i%64)/8)*100;}p.emplace_back(x,y);}if(shape<2){p[0]={INT32_MIN,0};p[1]={INT32_MAX,ybound};}if(shape>=2)std::shuffle(p.begin(),p.end(),rng);check(p);}
std::printf("PASS %ju exact boundary/axis/leaf-switch cases, %ju exhaustive pair evaluations\n",static_cast<std::uintmax_t>(checks),static_cast<std::uintmax_t>(pairs));}
