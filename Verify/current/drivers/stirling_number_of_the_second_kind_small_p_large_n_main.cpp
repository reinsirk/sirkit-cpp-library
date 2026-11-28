// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>

int main()
{
    int cases{};
    std::uint32_t prime{};
    verify_in >> cases >> prime;
    const auto table = sirkit::SmallPrimeStirlingNumbers::second_kind(prime);
    while (cases-- > 0)
    {
        std::uint64_t n{}, k{};
        verify_in >> n >> k;
        verify_out << table.get(n, k) << '\n';
    }
}
