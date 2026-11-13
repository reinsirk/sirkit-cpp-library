// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_ZETA_TEST_HEADER
#define SIRKIT_ZETA_TEST_HEADER "../../../Math/Zeta_Mebius.cpp"
#endif
#include SIRKIT_ZETA_TEST_HEADER
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>
std::uint64_t checks=0;
template<class T> T add(T a,T b){if constexpr(std::is_unsigned_v<T>)return static_cast<T>(std::uint64_t(a)+std::uint64_t(b));else return static_cast<T>(a+b);}
template<class T> T sub(T a,T b){if constexpr(std::is_unsigned_v<T>)return static_cast<T>(std::uint64_t(a)-std::uint64_t(b));else return static_cast<T>(a-b);}
template<class T> T mul(T a,T b){if constexpr(std::is_unsigned_v<T>)return static_cast<T>(std::uint64_t(a)*std::uint64_t(b));else return static_cast<T>(a*b);}
template<class T>std::vector<T> reference(std::vector<T> f,bool subset,bool minus){for(std::size_t bit=1;bit<f.size();bit*=2)for(std::size_t mask=0;mask<f.size();++mask)if(bool(mask&bit)==subset)f[mask]=minus?sub(f[mask],f[mask^bit]):add(f[mask],f[mask^bit]);return f;}
template<class T>void test(){std::mt19937_64 rng(49137+sizeof(T));
#ifdef SIRKIT_ZETA_NATIVE_TEST_REDUCED
 const std::vector<int> sizes{0,1,2,4,8,16,32,64,128,256,512,4096,65536};
#else
 const std::vector<int> sizes{0,1,2,4,8,16,32,64,128,256,512,1024,4096,65536,1048576};
#endif
 for(int size:sizes){const auto n=std::size_t(size);std::vector<T>a(n);for(auto& x:a)if constexpr(std::is_unsigned_v<T>)x=static_cast<T>(rng());else if constexpr(sizeof(T)>=4)x=static_cast<T>(int(rng()%3)-1);if constexpr(std::is_signed_v<T>&&sizeof(T)<4){for(std::size_t i=0;i<std::min(n,std::size_t{7});++i)a[rng()%n]=static_cast<T>(i&1?1:-1);}
 for(int mode=0;mode<4;++mode){auto actual=a;const bool subset=mode>=2,minus=mode&1;auto expected=reference(a,subset,minus);if(mode==0)superset_zeta_transform(actual);if(mode==1)superset_mobius_transform(actual);if(mode==2)subset_zeta_transform(actual);if(mode==3)subset_mobius_transform(actual);assert(actual==expected);++checks;}
 auto actual=a;subset_zeta_transform(actual);subset_mobius_transform(actual);assert(actual==a);actual=a;superset_zeta_transform(actual);superset_mobius_transform(actual);assert(actual==a);checks+=2;
 std::vector<T>x(n),y(n),wantand(n),wantor(n),wantsquareand(n),wantsquareor(n);std::vector<std::pair<std::size_t,T>>xs,ys;for(std::size_t i=0;i<std::min(n,std::size_t{7});++i){const auto index=i*n/std::min(n,std::size_t{7});const T value=static_cast<T>(i&1?1:-1);x[index]=value;xs.push_back({index,value});const auto other=n-1-index;y[other]=value;ys.push_back({other,value});}
 for(auto[i,a]:xs)for(auto[j,b]:ys){wantand[i&j]=add(wantand[i&j],mul(a,b));wantor[i|j]=add(wantor[i|j],mul(a,b));}for(auto[i,a]:xs)for(auto[j,b]:xs){wantsquareand[i&j]=add(wantsquareand[i&j],mul(a,b));wantsquareor[i|j]=add(wantsquareor[i|j],mul(a,b));}
 assert(bitwise_and_convolution(x,y)==wantand);assert(bitwise_or_convolution(x,y)==wantor);assert(bitwise_and_square(x)==wantsquareand);assert(bitwise_or_square(x)==wantsquareor);checks+=4;
 }}
int main(){test<std::uint8_t>();test<std::int8_t>();test<std::uint16_t>();test<std::int16_t>();test<std::uint32_t>();test<std::int32_t>();test<std::uint64_t>();test<std::int64_t>();std::cout<<"PASS native reference cases="<<checks<<'\n';}
