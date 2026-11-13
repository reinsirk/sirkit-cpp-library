// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
int main(){
    std::size_t n{},m{};if(!(verify_in>>n>>m))return 0;
    std::vector<std::uint32_t>a(n),b(m);
    for(auto& value:a)verify_in>>value;
    for(auto& value:b)verify_in>>value;
    const auto result=ConvolutionAnyMod(a,b,1000000007);
    for(std::size_t i=0;i<result.size();++i)verify_out<<result[i]<<(i+1==result.size()?'\n':' ');
}
