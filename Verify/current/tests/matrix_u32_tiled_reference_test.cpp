// SPDX-License-Identifier: CC0-1.0
#ifndef HEADER
#define HEADER "../../../Math/Matrix.cpp"
#endif
#include HEADER
#include HEADER
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>
std::uint64_t checks=0;
std::uint32_t dot(const std::uint32_t*a,const std::uint32_t*b,std::size_t k,std::size_t n,std::size_t i,std::size_t j){std::uint32_t sum=0;for(std::size_t x=0;x<k;++x)sum=static_cast<std::uint32_t>(std::uint64_t{a[i*k+x]}*b[x*n+j]+sum);return sum;}
void run(std::size_t m,std::size_t k,std::size_t n,std::mt19937_64&rng,bool sampled=false){Matrix<std::uint32_t>a(m,k),b(k,n);for(auto&x:a.values())x=static_cast<std::uint32_t>(rng());for(auto&x:b.values())x=static_cast<std::uint32_t>(rng());auto c=a*b;assert(c.rows()==m&&c.cols()==n);if(sampled){for(int q=0;q<80;++q){const auto i=rng()%m,j=rng()%n;assert(c(i,j)==dot(a.values().data(),b.values().data(),k,n,i,j));++checks;}}else for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<n;++j){assert(c(i,j)==dot(a.values().data(),b.values().data(),k,n,i,j));++checks;}}
int main(){std::mt19937_64 rng(731488);for(std::size_t k:std::array<std::size_t,16>{0,1,31,32,33,63,64,65,127,128,129,255,256,257,511,513})for(std::size_t n:std::array<std::size_t,14>{0,1,7,8,15,16,17,31,32,33,39,63,64,65})run(3,k,n,rng);
 for(std::size_t k:{63,64,65,127,128,129,255,256,257})for(std::size_t n:{4095,4096,4103})run(1,k,n,rng);
 for(const auto&shape:std::array<std::array<std::size_t,3>,7>{{{1,4096,4096},{4,4096,4103},{8,4096,4096},{4,65536,64},{2,129,32768},{4096,4,257},{128,65,4096}}})run(shape[0],shape[1],shape[2],rng,true);
 for(const auto&shape:std::array<std::array<std::size_t,3>,3>{{{5,65,4096},{9,129,4103},{17,257,1025}}})run(shape[0],shape[1],shape[2],rng);
 {Matrix<std::uint32_t> self(513,513);for(auto&x:self.values())x=static_cast<std::uint32_t>(rng());const auto original=self;self*=self;for(int q=0;q<160;++q){const auto i=rng()%513,j=rng()%513;assert(self(i,j)==dot(original.values().data(),original.values().data(),513,513,i,j));++checks;}}
#if defined(TEST_TILE) && !defined(SIRKIT_DISABLE_SIMD) && defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
 if(reinsirk::detail::matrix_u32_avx2_available())for(std::size_t k:std::array<std::size_t,14>{1,31,32,33,63,64,65,127,128,129,255,256,257,513})for(std::size_t n:std::array<std::size_t,12>{1,7,8,15,16,17,31,32,33,63,64,65}){constexpr std::size_t m=3;std::vector<std::uint32_t>a(m*k+9),b(k*n+9),c(m*n+12,0xdeadbeef);for(auto&x:a)x=static_cast<std::uint32_t>(rng());for(auto&x:b)x=static_cast<std::uint32_t>(rng());auto*output=c.data()+5;for(std::size_t i=0;i<m*n;++i)output[i]=0x13572468;reinsirk::detail::matrix_u32_tiled_avx2(a.data()+1,b.data()+3,output,m,k,n);for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<n;++j){assert(output[i*n+j]==dot(a.data()+1,b.data()+3,k,n,i,j));++checks;}for(std::size_t i=0;i<5;++i)assert(c[i]==0xdeadbeef);for(std::size_t i=m*n+5;i<c.size();++i)assert(c[i]==0xdeadbeef);}
#endif
 std::printf("matrix robust full-oracle/boundaries/misaligned+canary/sampled-large checks=%ju\n",static_cast<std::uintmax_t>(checks));}
