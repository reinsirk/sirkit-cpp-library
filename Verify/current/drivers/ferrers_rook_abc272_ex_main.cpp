// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::uint32_t n{};
    verify_in >> n;
    std::vector<std::uint32_t> heights(n);
    for (auto &height : heights)
    {
        verify_in >> height;
        ++height;
    }
    const auto hits = sirkit::ferrers_hit_counts(heights);
    std::uint32_t even_flips = 0;
    for (std::size_t i = 0; i < hits.size(); i += 2)
    {
        even_flips = sirkit::detail::fps_add(even_flips, hits[i]);
    }
    const auto multiply = sirkit::detail::fps_multiply;
    std::uint32_t factorial_before_n = 1;
    for (std::uint32_t i = 1; i < n; ++i)
    {
        factorial_before_n = multiply(factorial_before_n, i);
    }
    const auto inverse = sirkit::detail::fps_power(factorial_before_n, sirkit::FormalPowerSeries::modulus() - 2);
    verify_out << multiply(even_flips, inverse) << '\n';
}
