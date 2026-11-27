// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    std::size_t tests{};
    verify_in >> tests;
    std::vector<std::pair<std::int64_t, std::int64_t>> vertices;
    while (tests-- != 0)
    {
        std::size_t n{};
        verify_in >> n;
        vertices.resize(n);
        for (auto &[x, y] : vertices)
        {
            verify_in >> x >> y;
        }
        const auto neighbors = sirkit::all_furthest_neighbors(vertices);
        for (std::size_t i = 0; i < n; ++i)
        {
            verify_out << neighbors[i] << (i + 1 == n ? '\n' : ' ');
        }
    }
}
