// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    int tests = 0;
    verify_in >> tests;
    while (tests-- > 0)
    {
        std::uint32_t k = 0, y = 0, p = 0;
        verify_in >> k >> y >> p;
        const auto root = modular_kth_root(k, y, p);
        verify_out << (root ? static_cast<std::int64_t>(*root) : -1) << '\n';
    }
}
