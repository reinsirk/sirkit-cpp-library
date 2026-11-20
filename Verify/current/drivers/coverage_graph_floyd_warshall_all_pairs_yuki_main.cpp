// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>
int main() {
    int n{}, m{}; verify_in >> n >> m;
    std::vector distances(n, std::vector<std::int64_t>(n, kInf));
    for (int v = 0; v < n; ++v) distances[v][v] = 0;
    while (m--) { int u{}, v{}; std::int64_t weight{}; verify_in >> u >> v >> weight; --u; --v; distances[u][v] = std::min(distances[u][v], weight); }
    FloydWarshall(distances);
    for (const auto& row : distances) {
        std::int64_t answer = 0;
        for (auto value : row) if (value != kInf) answer += value;
        verify_out << answer << '\n';
    }
}
