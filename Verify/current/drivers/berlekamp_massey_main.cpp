// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
int main() {
    std::size_t n{};if(!(verify_in>>n))return 0;
    std::vector<std::uint32_t> sequence(n);
    for(auto&value:sequence)verify_in>>value;
    const auto coefficients=BerlekampMassey(sequence);
    verify_out<<coefficients.size()<<'\n';
    for(std::size_t i=0;i<coefficients.size();++i)verify_out<<coefficients[i]<<(i+1==coefficients.size()?'\n':' ');
    if(coefficients.empty())verify_out<<'\n';
}
