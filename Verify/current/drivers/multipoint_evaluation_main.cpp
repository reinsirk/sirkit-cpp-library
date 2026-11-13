// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
int main(){
    std::size_t n{},m{};if(!(verify_in>>n>>m))return 0;
    std::vector<std::uint32_t> coefficients(n),points(m);
    for(auto& value:coefficients)verify_in>>value;
    for(auto& value:points)verify_in>>value;
    const auto result=MultipointEvaluate(coefficients,points);
    for(std::size_t i=0;i<result.size();++i)verify_out<<result[i]<<(i+1==result.size()?'\n':' ');
}
