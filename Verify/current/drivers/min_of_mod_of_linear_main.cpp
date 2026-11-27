// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>

int main()
{
    int queries{};
    verify_in >> queries;
    while (queries-- > 0)
    {
        std::int64_t n{}, m{}, a{}, b{};
        verify_in >> n >> m >> a >> b;
        verify_out << min_of_mod_of_linear(n, m, a, b) << '\n';
    }
}
