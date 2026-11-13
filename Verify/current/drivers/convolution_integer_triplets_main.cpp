// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
int main(){
    std::size_t n{};if(!(verify_in>>n))return 0;
    std::vector<std::size_t> values(n);for(auto& value:values)verify_in>>value;
    const auto maximum=*std::max_element(values.begin(),values.end());
    std::vector<std::int64_t> indicator(maximum+1);
    for(const auto value:values)indicator[value]=1;
    CRTConvolution convolution;
    const auto counts=convolution.integer_square(indicator);
    std::int64_t answer=0;
    for(const auto value:values)answer+=(counts[2*value]-1)/2;
    verify_out<<answer<<'\n';
}
