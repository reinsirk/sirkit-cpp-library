// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
int main() {
    int n{}; verify_in >> n;
    std::vector<DynamicBitset> graph;
    graph.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        std::string row; verify_in >> row;
        graph.emplace_back(row);
    }
    std::int64_t total = 0;
    for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) {
        if (graph[static_cast<std::size_t>(u)].test(static_cast<std::size_t>(n - 1 - v)))
            total += static_cast<std::int64_t>(graph[static_cast<std::size_t>(u)].intersection_count(graph[static_cast<std::size_t>(v)]));
    }
    verify_out << total / 3 << '\n';
}
