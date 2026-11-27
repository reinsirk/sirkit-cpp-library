// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n;
    verify_in >> n;
    std::vector<std::vector<std::uint32_t>> factors(n);
    for (auto &factor : factors)
    {
        std::size_t degree;
        verify_in >> degree;
        factor.resize(degree + 1);
        for (auto &coefficient : factor)
        {
            verify_in >> coefficient;
        }
    }
    const auto result = polynomial_sequence_product(std::move(factors));
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        verify_out << result[i] << (i + 1 == result.size() ? '\n' : ' ');
    }
}
