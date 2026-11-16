// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstddef>
#include <vector>

int main() {
    int n{}, k{};
    verify_in >> n >> k;
    Graph graph(n);
    while (k--) {
        int row{}, column{};
        verify_in >> row >> column;
        graph[row - 1].push_back(column - 1);
    }
    const auto order = TopologicalSort(graph);
    if (order.size() != static_cast<std::size_t>(n)) {
        verify_out << -1 << '\n';
        return 0;
    }
    std::vector<int> longest(n, 1);
    int answer = 1;
    for (int vertex : order) {
        answer = std::max(answer, longest[vertex]);
        for (int next : graph[vertex])
            longest[next] = std::max(longest[next], longest[vertex] + 1);
    }
    verify_out << answer << '\n';
}
