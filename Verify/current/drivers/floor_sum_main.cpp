// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>

int main()
{
    int q{};
    verify_in >> q;
    for (int i = 0; i < q; ++i)
    {
        std::int64_t n{}, m{}, a{}, b{};
        verify_in >> n >> m >> a >> b;
        verify_out << floor_sum(n, m, a, b) << '\n';
    }
}
