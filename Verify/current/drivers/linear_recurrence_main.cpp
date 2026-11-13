// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
int main() {
    std::size_t d{};std::uint64_t k{};if(!(verify_in>>d>>k))return 0;
    std::vector<std::uint32_t> initial(d),coefficients(d);
    for(auto&value:initial)verify_in>>value;
    for(auto&value:coefficients)verify_in>>value;
    verify_out<<LinearRecurrenceTerm(initial,coefficients,k)<<'\n';
}
