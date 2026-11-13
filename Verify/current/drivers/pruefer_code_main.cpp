// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <utility>
#include <vector>
int main() {
    VerifyInput input;
    VerifyOutput output;
    int n = 0, m = 0; input >> n >> m;
    std::vector<int> degrees(static_cast<std::size_t>(n));
    for (auto& degree : degrees) input >> degree;
    std::vector<std::pair<int,int>> edges(static_cast<std::size_t>(m));
    for (auto& [u, v] : edges) { input >> u >> v; --u; --v; }
    auto added = complete_tree_with_degrees(degrees, edges);
    if (!added) { output << -1 << '\n'; return 0; }
    for (const auto& [u, v] : *added) output << u + 1 << ' ' << v + 1 << '\n';
}
