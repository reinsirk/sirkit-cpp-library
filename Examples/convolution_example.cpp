// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <iostream>
#include <vector>
#include "../Math/Convolution.cpp"
int main(){
    CRTConvolution convolution;
    const std::vector<int> a{1,-2,3},b{4,5};
    for(auto value:convolution.modular(a,b,1000000007))std::cout<<value<<' ';
    std::cout<<'\n';
    const std::vector<std::int64_t> x{-1,2,3};
    for(auto value:convolution.integer_square(x))std::cout<<value<<' ';
    std::cout<<'\n';
    std::cout<<convolution.modular_square(a,1000000007)[0]<<' '
             <<convolution.integer(x,x)[0]<<' '
             <<ConvolutionAnyMod(a,b,7)[0]<<' '
             <<ConvolutionInt64(x,x)[0]<<'\n';
    std::cout<<CRTConvolution::max_length()<<'\n';
    convolution.prepare(1024);
    auto independent=convolution;
    convolution.clear_cache();
    std::cout<<(independent.roots_capacity()>0)<<'\n';
}
