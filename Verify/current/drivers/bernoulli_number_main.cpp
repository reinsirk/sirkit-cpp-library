// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>

int main()
{
    std::size_t n;
    verify_in >> n;
    const auto numbers = bernoulli_numbers(n);
    for (std::size_t i = 0; i < numbers.size(); ++i)
    {
        verify_out << numbers[i] << (i == n ? '\n' : ' ');
    }
}
