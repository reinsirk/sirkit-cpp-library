// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int groups{}, path_size{};
    verify_in >> groups >> path_size;
    const int n = groups * path_size;
    std::vector<std::pair<int, int>> edges(n - 1);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const auto paths = sirkit::partition_forest_into_equal_paths(n, edges, path_size);
    verify_out << (paths ? "Yes\n" : "No\n");
}
