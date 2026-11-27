// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    std::size_t n;
    verify_in >> n;
    std::vector<std::uint32_t> coefficients(n);
    for (auto &value : coefficients)
    {
        verify_in >> value;
    }
    const auto sum = PolynomialPrefixSum(FormalPowerSeries(std::move(coefficients)));
    for (std::size_t i = 0; i <= n; ++i)
    {
        verify_out << sum.get(i) << (i == n ? '\n' : ' ');
    }
}
