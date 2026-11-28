// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <vector>

int main()
{
    std::size_t size{};
    verify_in >> size;
    std::vector<std::array<int, 3>> coordinates(size);
    for (auto &[x, y, z] : coordinates)
    {
        verify_in >> x >> y >> z;
    }
    const auto cost_at = [&](std::size_t u, std::size_t v)
    {
        const auto &[x, y, z] = coordinates[u];
        const auto &[next_x, next_y, next_z] = coordinates[v];
        return std::abs(x - next_x) + std::abs(y - next_y) + std::max(0, next_z - z);
    };
    verify_out << sirkit::shortest_hamiltonian_cycle(size, 0, cost_at)->cost << '\n';
}
