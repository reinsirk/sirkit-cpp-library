namespace sirkit {}
using namespace sirkit;
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
#ifndef HEADER
#define HEADER "../../../Math/Convolution.cpp"
#endif
#include HEADER
int main(){
 CRTConvolution c;constexpr std::size_t n=1<<17,m=1<<16;
 std::vector<std::int64_t>a(n,-3),b(m,7);
 const auto integer=c.integer(a,b);
 for(std::size_t i=0;i<integer.size();++i){const auto count=std::min(std::min(i+1,integer.size()-i),std::min(n,m));if(integer[i]!=-21*static_cast<std::int64_t>(count))throw std::runtime_error("large integer mismatch");}
 const std::uint32_t mod=UINT32_MAX,value=UINT32_MAX-1;
 std::vector<std::uint32_t>x(n,value),y(m,value);
 const auto modular=c.modular(x,y,mod);
 for(std::size_t i=0;i<modular.size();++i){const auto count=std::min(std::min(i+1,modular.size()-i),std::min(n,m));if(modular[i]!=count)throw std::runtime_error("large maxmod mismatch");}
 const auto square=c.modular_square(x,mod);
 for(std::size_t i=0;i<square.size();++i){const auto count=std::min(i+1,square.size()-i);if(square[i]!=count)throw std::runtime_error("large square mismatch");}
 std::cout<<"PASS large triangular signed/max32mod/square and cache reuse\n";
}
