// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<int> a(n), b(m);
    for (int &x : a)
    {
        verify_in >> x;
    }
    for (int &x : b)
    {
        verify_in >> x;
    }
    const auto sums = min_plus_convolution_convex_convex(a, b);
    for (std::size_t i = 0; i < sums.size(); ++i)
    {
        verify_out << sums[i] << (i + 1 == sums.size() ? '\n' : ' ');
    }
}
