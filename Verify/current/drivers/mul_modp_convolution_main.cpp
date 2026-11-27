// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n;
    verify_in >> n;
    std::vector<std::uint32_t> a(n), b(n);
    for (auto &x : a)
    {
        verify_in >> x;
    }
    for (auto &x : b)
    {
        verify_in >> x;
    }
    const auto product = multiplicative_convolution_prime(a, b);
    for (std::size_t i = 0; i < n; ++i)
    {
        verify_out << product[i] << (i + 1 == n ? '\n' : ' ');
    }
}
