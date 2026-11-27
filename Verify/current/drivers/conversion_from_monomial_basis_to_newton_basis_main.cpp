// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n;
    verify_in >> n;
    std::vector<std::uint32_t> coefficients(n), points(n);
    for (auto &c : coefficients)
    {
        verify_in >> c;
    }
    for (auto &p : points)
    {
        verify_in >> p;
    }
    if (!points.empty())
    {
        points.pop_back();
    }
    const auto values = monomial_to_newton_basis(coefficients, points);
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        verify_out << values[i] << (i + 1 == values.size() ? '\n' : ' ');
    }
    if (values.empty())
    {
        verify_out << '\n';
    }
}
