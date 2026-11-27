// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>

int main()
{
    int queries{};
    verify_in >> queries;
    while (queries-- > 0)
    {
        std::uint64_t n{};
        unsigned k{};
        verify_in >> n >> k;
        verify_out << integer_kth_root(n, k) << '\n';
    }
}
