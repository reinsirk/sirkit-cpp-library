// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>

int main()
{
    std::size_t n;
    verify_in >> n;
    const auto result = partition_numbers(n);
    for (std::size_t i = 0; i <= n; ++i)
    {
        verify_out << result[i] << (i == n ? '\n' : ' ');
    }
}
