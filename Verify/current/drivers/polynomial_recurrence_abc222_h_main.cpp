// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main()
{
    std::uint64_t n{};
    verify_in >> n;
    const auto power = static_cast<std::int64_t>(2 * n);
    const std::array<std::int64_t, 2> initial{1, 3 * power};
    const std::vector<std::vector<std::int64_t>> coefficients{
        {-2 * power, 1},
        {3 * (1 - power), 3},
        {2, 1}};
    const auto count = sirkit::polynomial_recurrence_term(initial, coefficients, n - 1);
    verify_out << sirkit::detail::ConsecutiveMultiply(count, sirkit::detail::ConsecutiveInverse(static_cast<std::uint32_t>(n))) << '\n';
}
