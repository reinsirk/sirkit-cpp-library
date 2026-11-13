// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
int main(){
    std::size_t n{};std::uint32_t shift{};if(!(verify_in>>n>>shift))return 0;
    std::vector<std::uint32_t> coefficients(n);
    for(auto& value:coefficients)verify_in>>value;
    const auto result=TaylorShift(coefficients,shift);
    for(std::size_t i=0;i<result.size();++i)verify_out<<result.get(i)<<(i+1==result.size()?'\n':' ');
}
