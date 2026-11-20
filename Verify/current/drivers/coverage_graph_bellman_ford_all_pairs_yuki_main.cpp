// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>
int main() {
    int n{}, m{}; verify_in >> n >> m;
    WGraph graph(n);
    while (m--) { int u{}, v{}; std::int64_t weight{}; verify_in >> u >> v >> weight; graph[u - 1].emplace_back(v - 1, weight); }
    std::vector<std::int64_t> distances;
    for (int start = 0; start < n; ++start) {
        BellmanFord(graph, distances, start);
        std::int64_t answer = 0;
        for (auto value : distances) if (value != kInf) answer += value;
        verify_out << answer << '\n';
    }
}
