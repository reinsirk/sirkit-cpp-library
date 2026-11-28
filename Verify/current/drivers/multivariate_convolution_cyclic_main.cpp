// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t k{};
    std::uint32_t prime{};
    verify_in >> prime >> k;
    std::vector<std::size_t> dimensions(k);
    std::size_t size = 1;
    for (auto &length : dimensions)
    {
        verify_in >> length;
        size *= length;
    }
    std::vector<std::uint32_t> a(size), b(size);
    for (auto &entry : a)
    {
        verify_in >> entry;
    }
    for (auto &entry : b)
    {
        verify_in >> entry;
    }
    const auto result = sirkit::cyclic_multivariate_convolution(dimensions, a, b, prime);
    for (std::size_t i = 0; i < size; ++i)
    {
        verify_out << result[i] << (i + 1 == size ? '\n' : ' ');
    }
}
