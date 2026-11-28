// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t degree{};
    std::uint32_t prime{};
    verify_in >> degree >> prime;
    std::vector<std::uint32_t> coefficients(degree + 1);
    for (auto &coefficient : coefficients)
    {
        verify_in >> coefficient;
    }
    const auto result = sirkit::factor_polynomial(coefficients, prime);
    verify_out << result.factors.size() << '\n';
    for (const auto &factor : result.factors)
    {
        verify_out << factor.multiplicity << ' ' << factor.coefficients.size() - 1;
        for (const auto coefficient : factor.coefficients)
        {
            verify_out << ' ' << coefficient;
        }
        verify_out << '\n';
    }
}
