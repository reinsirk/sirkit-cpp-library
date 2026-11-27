// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n, m;
    verify_in >> n >> m;
    std::vector<std::int64_t> a(n), b(m);
    for (auto &x : a)
    {
        verify_in >> x;
    }
    for (auto &x : b)
    {
        verify_in >> x;
    }
    const auto result = min_plus_convolution_concave_arbitrary(a, b);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        verify_out << result[i] << (i + 1 == result.size() ? '\n' : ' ');
    }
}
