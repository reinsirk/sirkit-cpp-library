// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    std::size_t degree{};
    verify_in >> degree;
    std::vector<std::uint32_t> coefficients(degree + 1);
    for (auto &coefficient : coefficients)
    {
        verify_in >> coefficient;
    }
    const auto roots = sirkit::polynomial_roots(sirkit::FormalPowerSeries(std::move(coefficients)));
    verify_out << roots.size() << '\n';
    for (std::size_t i = 0; i < roots.size(); ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << roots[i];
    }
    verify_out << '\n';
}
