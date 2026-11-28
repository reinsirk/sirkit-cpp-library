// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{};
    std::uint64_t k{};
    verify_in >> n >> k;
    std::vector<int> next(n);
    std::vector<std::uint64_t> candies(n);
    for (int v = 0; v < n; ++v)
    {
        verify_in >> candies[v];
        next[v] = static_cast<int>((candies[v] + v) % n);
    }
    const sirkit::FunctionalGraphFold<std::uint64_t> moves(next, candies, 0);
    verify_out << moves.walk(0, k).second << '\n';
}
