// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef HEADER
#define HEADER "../../../Math/Matrix.cpp"
#endif
#include HEADER
#include HEADER
#include <array>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <type_traits>
static std::uint64_t checks=0;
void require(bool value){++checks;if(!value)throw std::runtime_error("matrix SIMD mismatch");}
template<class T>void test(std::size_t m,std::size_t k,std::size_t n,std::mt19937_64&rng){
 Matrix<T>a(m,k),b(k,n);for(auto&x:a.values()){if constexpr(std::is_same_v<T,std::uint32_t>)x=static_cast<T>(rng());else if constexpr(std::is_signed_v<T>)x=static_cast<T>(static_cast<int>(rng()%3)-1);else x=static_cast<T>(rng()%2);}for(auto&x:b.values()){if constexpr(std::is_same_v<T,std::uint32_t>)x=static_cast<T>(rng());else if constexpr(std::is_signed_v<T>)x=static_cast<T>(static_cast<int>(rng()%3)-1);else x=static_cast<T>(rng()%2);}
 const auto c=a*b;for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<n;++j){if constexpr(std::is_same_v<T,std::uint32_t>){std::uint32_t sum=0;for(std::size_t p=0;p<k;++p)sum=static_cast<std::uint32_t>(std::uint64_t{a(i,p)}*b(p,j)+sum);require(c(i,j)==sum);}else{std::int64_t sum=0;for(std::size_t p=0;p<k;++p)sum+=static_cast<std::int64_t>(a(i,p))*b(p,j);require(c(i,j)==static_cast<T>(sum));}}
 if(m==k&&k==n){auto self=a;self*=self;const auto separate=a*a;require(self==separate);}
}
int main(){std::mt19937_64 rng(0x887ab);
 for(const auto dimensions:std::array<std::array<std::size_t,3>,22>{{{0,0,0},{0,17,33},{33,0,33},{1,1,1},{1,63,16},{1,64,16},{3,17,31},{4,16,16},{7,9,64},{8,8,33},{16,16,16},{17,31,15},{31,17,31},{31,33,32},{33,31,33},{63,65,63},{64,64,64},{65,33,65},{192,31,17},{16,256,384},{384,256,16},{192,192,192}}})test<std::uint32_t>(dimensions[0],dimensions[1],dimensions[2],rng);
 for(int rep=0;rep<100;++rep){const auto m=static_cast<std::size_t>(rng()%80),k=static_cast<std::size_t>(rng()%80),n=static_cast<std::size_t>(rng()%80);test<std::int32_t>(m,k,n,rng);test<std::uint16_t>(m,k,n,rng);test<std::int16_t>(m,k,n,rng);test<std::uint64_t>(m,k,n,rng);}
 std::cout<<"PASS matrix SIMD checks="<<checks<<"; uint32 wrap, threshold/tail/misalignment, signed16/32 unsigned16/64 and alias\n";
}
