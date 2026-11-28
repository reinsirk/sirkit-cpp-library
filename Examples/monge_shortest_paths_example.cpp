// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Monge_Shortest_Paths.cpp"

#include <cassert>
#include <cstdint>

int main()
{
    const auto result = sirkit::monge_shortest_paths(5, [](int from, int to) {
        const auto length = static_cast<std::int64_t>(to - from);
        return length * length + 3;
    });
    assert(result.distances[4] == 14);
    assert(result.predecessors[4] == 2);
}
