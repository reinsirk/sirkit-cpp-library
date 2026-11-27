// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    unsigned count;
    verify_in >> count;
    for (unsigned i = 0; i < count; ++i)
    {
        std::uint64_t p;
        verify_in >> p;
        verify_out << primitive_root(p) << '\n';
    }
}
