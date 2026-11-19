// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_ANGLE_TEST_SOURCE
#define SIRKIT_ANGLE_TEST_SOURCE "../../../Geometry/angle_sort.cpp"
#endif
#include SIRKIT_ANGLE_TEST_SOURCE
#include SIRKIT_ANGLE_TEST_SOURCE
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <bit>
#include <cstdio>
#include <cstdlib>
#include <random>
using Point=std::pair<std::int64_t,std::int64_t>;using Big=boost::multiprecision::cpp_int;
std::uint64_t checks=0;
void require(bool p){++checks;if(!p)std::abort();}
int refcross(Point a,Point b){Big z=Big(a.first)*b.second-Big(a.second)*b.first;return (z>0)-(z<0);}
bool refless(Point a,Point b){if(a==Point(0,0))return b!=Point(0,0);if(b==Point(0,0))return false;bool ah=a.second<0||(a.second==0&&a.first<0),bh=b.second<0||(b.second==0&&b.first<0);if(ah!=bh)return ah<bh;return refcross(a,b)>0;}
void verify(const std::vector<Point>&input){for(bool desc:{false,true}){auto p=input;angle_sort(p,desc);require(p.size()==input.size());auto a=p,b=input;std::sort(a.begin(),a.end());std::sort(b.begin(),b.end());require(a==b);for(std::size_t i=1;i<p.size();++i)require(desc?!refless(p[i-1],p[i]):!refless(p[i],p[i-1]));auto original=input;auto ids=angle_sort_indices(input,desc);require(input==original&&ids.size()==input.size());std::vector<bool>seen(input.size());for(std::size_t i=0;i<ids.size();++i){require(ids[i]<seen.size()&&!seen[ids[i]]);seen[ids[i]]=true;if(i)require(desc?!refless(input[ids[i-1]],input[ids[i]]):!refless(input[ids[i]],input[ids[i-1]]));}}}
void adaptive_boundaries() {
    constexpr std::size_t n = 4096;
    std::vector<Point> sorted;
    for (std::size_t i = 0; i < n; ++i) sorted.emplace_back(1, static_cast<std::int64_t>(i + 10));
    const auto swap_preceding = [](auto& points, std::size_t at) { std::swap(points[at - 1], points[at]); };
    for (std::size_t count : {1, 2}) {
        auto forward = sorted;
        auto backward = sorted;
        for (std::size_t sample = 1; sample <= count; ++sample) {
            swap_preceding(forward, sample * (n / 17));
            swap_preceding(backward, n - 1 - sample * (n / 64));
        }
        verify(forward);
        verify(backward);
    }
    auto smallest_near_start = sorted;
    smallest_near_start[4] = Point{1, 1};
    verify(smallest_near_start);
    auto smallest_in_tail = sorted;
    smallest_in_tail[n - 3] = Point{1, 1};
    smallest_in_tail[n - 2] = Point{1, 2};
    verify(smallest_in_tail);
}
int main(){std::mt19937_64 rng(781922);std::array<std::int64_t,11>corner{INT64_MIN,INT64_MIN+1,-(std::int64_t(1)<<32),INT32_MIN,-1,0,1,INT32_MAX,std::int64_t(1)<<32,INT64_MAX-1,INT64_MAX};for(int i=0;i<100000;++i){auto point=[&]{if(i%3==0)return Point(corner[rng()%corner.size()],corner[rng()%corner.size()]);return Point(std::bit_cast<std::int64_t>(rng()),std::bit_cast<std::int64_t>(rng()));};auto a=point(),b=point(),c=point();bool ab=refless(a,b),ba=refless(b,a);require(angle_cross_sign(a,b)==refcross(a,b));require(arccmp_less(a,b)==ab&&AngleLess{}(a,b)==ab);require(arccmp_g(a,b)==ba&&AngleGreater{}(a,b)==ba);require(arccmp_less_eq(a,b)==!ba&&arccmp_g_eq(a,b)==!ab);require(same_angle(a,b)==(!ab&&!ba));if(ab&&refless(b,c))require(arccmp_less(a,c));if(!ab&&!ba&&!refless(b,c)&&!refless(c,b))require(!arccmp_less(a,c)&&!arccmp_less(c,a));}
for(int k=0;k<1500;++k){std::vector<Point>p;int n=k%66;for(int i=0;i<n;++i){if(k%3==0)p.emplace_back(corner[rng()%corner.size()],corner[rng()%corner.size()]);else if(k%3==1)p.emplace_back(std::bit_cast<std::int64_t>(rng()),std::bit_cast<std::int64_t>(rng()));else p.emplace_back(int(rng()%33)-16,int(rng()%33)-16);}verify(p);}
for(int n:{31,32,33,2047,2048,2049,4096})for(int shape=0;shape<10;++shape){std::vector<Point>p;for(int i=0;i<n;++i){if(shape==0)p.emplace_back(0,0);else if(shape==1)p.emplace_back(i+1,(i+1)*3);else if(shape==9)p.emplace_back(std::bit_cast<std::int64_t>(rng()),std::bit_cast<std::int64_t>(rng()));else p.emplace_back(std::int32_t(rng()),std::int32_t(rng()));}if(shape>=2&&shape<=8){std::sort(p.begin(),p.end(),refless);if(shape==3)std::reverse(p.begin(),p.end());if(shape==4)std::rotate(p.begin(),p.begin()+n/2,p.end());if(shape==5&&n>3)std::swap(p[n-3],p[n-2]);if(shape==6)std::shuffle(p.begin()+n*9/10,p.end(),rng);if(shape==7&&n>3)std::rotate(p.begin()+n/10,p.begin()+n/10+1,p.end());if(shape==8){std::shuffle(p.begin()+n*9/10,p.end(),rng);std::sort(p.end()-std::min(n,16),p.end(),refless);}}verify(p);}
for(int n:{2048,4096})for(bool desc:{false,true})for(int kind=0;kind<4;++kind){std::vector<Point>p;for(int i=0;i<n;++i)p.emplace_back(std::bit_cast<std::int64_t>(rng()),std::bit_cast<std::int64_t>(rng()));std::sort(p.begin(),p.end(),[desc](Point a,Point b){return desc?refless(b,a):refless(a,b);});if(kind==0)std::rotate(p.begin()+n/10,p.begin()+n/10+1,p.end());if(kind==1)std::rotate(p.begin()+n/10,p.begin()+n/10+1,p.begin()+n/2);if(kind==2){for(int k=1;k<5;++k)std::swap(p[n-k*100],p[n-k*100+1]);}if(kind==3)std::swap(p[n-3],p[n-2]);verify(p);}
adaptive_boundaries();
std::printf("PASS %ju exact cpp_int cross/weak ordering/permutation/index/input-preservation cases\n",static_cast<std::uintmax_t>(checks));}
