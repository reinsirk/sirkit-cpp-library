// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
int main(){
    std::size_t n{};if(!(verify_in>>n))return 0;
    std::vector<std::uint32_t> points(n),values(n);
    for(auto& value:points)verify_in>>value;
    for(auto& value:values)verify_in>>value;
    const auto result=PolynomialInterpolate(points,values);
    for(std::size_t i=0;i<result.size();++i)verify_out<<result.get(i)<<(i+1==result.size()?'\n':' ');
}
