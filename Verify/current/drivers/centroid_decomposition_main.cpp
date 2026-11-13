// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
int main() {
    VerifyInput input;
    VerifyOutput output;
    int n = 0; input >> n;
    Graph graph(static_cast<std::size_t>(n));
    for (int i = 0; i < n - 1; ++i) {
        int u = 0, v = 0; input >> u >> v; --u; --v;
        graph[static_cast<std::size_t>(u)].push_back(v);
        graph[static_cast<std::size_t>(v)].push_back(u);
    }
    CentroidDecomposition decomposition(graph, false);
    for (int vertex = 0; vertex < n; ++vertex) {
        if (vertex != 0) output << ' ';
        const int p = decomposition.parent(vertex);
        output << (p == -1 ? -1 : p + 1);
    }
    output << '\n';
}
