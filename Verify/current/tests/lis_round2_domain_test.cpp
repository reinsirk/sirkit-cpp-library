// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/LIS.cpp"
#include <cassert>
#include <random>
#include <cstdio>
template<bool Strict>void check(const std::vector<std::int64_t>&v){std::vector<std::int64_t>d;for(auto x:v){auto at=Strict?std::lower_bound(d.begin(),d.end(),x):std::upper_bound(d.begin(),d.end(),x);if(at==d.end())d.push_back(x);else*at=x;}auto ix=LISIndices<Strict>(v);auto seq=LISSequence<Strict>(v);assert(ix.size()==d.size()&&seq.size()==ix.size());for(std::size_t i=0;i<ix.size();++i){assert(ix[i]<v.size()&&seq[i]==v[ix[i]]);if(i)assert(ix[i-1]<ix[i]&&(Strict?v[ix[i-1]]<v[ix[i]]:v[ix[i-1]]<=v[ix[i]]));}}
int main(){std::mt19937_64 r(82841);unsigned count=0;for(std::size_t n:{std::size_t(65535),std::size_t(65536),std::size_t(65537),std::size_t(200000)})for(int dom:{2,3,4,8,16,256})for(int shape=0;shape<3;++shape){std::vector<std::int64_t>a(n);for(std::size_t i=0;i<n;++i)a[i]=shape==1&&i>n/3?std::int64_t(1000000+i):shape==2&&i>n/3?-std::int64_t(i):std::int64_t(r()%dom);check<true>(a);check<false>(a);count+=2;}std::printf("%u domain/cache exit cases passed\n",count);}
