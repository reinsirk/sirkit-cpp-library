// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>

int main()
{
    int queries{};
    verify_in >> queries;
    while (queries-- > 0)
    {
        std::uint32_t base{}, target{}, modulus{};
        verify_in >> base >> target >> modulus;
        const auto exponent = discrete_logarithm_mod(base, target, modulus);
        verify_out << (exponent ? static_cast<std::int64_t>(*exponent) : -1) << '\n';
    }
}
