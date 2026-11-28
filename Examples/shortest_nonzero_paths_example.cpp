// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Shortest_Nonzero_Paths.cpp"

#include <cassert>
#include <functional>
#include <vector>

int main()
{
    const std::vector<sirkit::GroupPathEdge<unsigned>> edges{{0, 1, 2, 0}, {1, 2, 2, 0}, {0, 2, 7, 1}};
    const auto distances = sirkit::shortest_nonzero_paths(3, edges, 0, 0u, std::bit_xor<>{}, std::identity{});
    assert(!distances[0]);
    assert(distances[1] == 9);
    assert(distances[2] == 7);
}
