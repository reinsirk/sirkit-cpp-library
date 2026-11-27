// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    int q;
    verify_in >> q;
    while (q-- > 0)
    {
        std::uint64_t base, height;
        std::uint32_t mod;
        verify_in >> base >> height >> mod;
        verify_out << tetration_mod(base, height, mod) << '\n';
    }
}
