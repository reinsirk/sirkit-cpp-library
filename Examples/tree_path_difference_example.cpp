// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Path_Difference.cpp"

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<int, int>> edges{{0, 1}, {1, 2}, {1, 3}};
    sirkit::TreePathDifference<> paths(4, edges);
    paths.add_vertex_path(2, 3, 4);
    paths.add_edge_path(0, 2, 3);
    assert((paths.vertex_values() == std::vector<std::int64_t>{0, 4, 4, 4}));
    assert((paths.edge_values() == std::vector<std::int64_t>{3, 3, 0}));
}
