// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t k;
    verify_in >> k;
    std::vector<std::size_t> dimensions(k);
    std::size_t n = 1;
    for (auto &width : dimensions)
    {
        verify_in >> width;
        n *= width;
    }
    std::vector<std::uint32_t> a(n), b(n);
    for (auto &x : a)
    {
        verify_in >> x;
    }
    for (auto &x : b)
    {
        verify_in >> x;
    }
    const auto c = multivariate_convolution(dimensions, a, b);
    for (std::size_t i = 0; i < n; ++i)
    {
        verify_out << c[i] << (i + 1 == n ? '\n' : ' ');
    }
}
