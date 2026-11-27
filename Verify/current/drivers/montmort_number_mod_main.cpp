// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>

int main()
{
    std::size_t n;
    verify_in >> n;
    std::uint32_t mod;
    verify_in >> mod;
    const auto result = derangement_numbers(n, mod);
    for (std::size_t i = 1; i <= n; ++i)
    {
        verify_out << result[i] << (i == n ? '\n' : ' ');
    }
}
