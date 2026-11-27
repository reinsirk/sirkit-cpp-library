// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>

int main()
{
    int queries{};
    verify_in >> queries;
    while (queries-- > 0)
    {
        std::uint64_t limit{};
        SternBrocotTree::Fraction fraction{};
        verify_in >> limit >> fraction.first >> fraction.second;
        const auto [lower, upper] = SternBrocotTree::approximate(fraction, limit, limit);
        verify_out << lower.first << ' ' << lower.second << ' ' << upper.first << ' ' << upper.second << '\n';
    }
}
