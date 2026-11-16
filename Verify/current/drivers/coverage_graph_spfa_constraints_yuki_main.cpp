// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <array>
#include <cstdint>
#include <vector>
int main() {
    std::array<std::int64_t, 5> capacity{};
    std::int64_t total = 0;
    for (auto& value : capacity) { verify_in >> value; total += value; }
    const auto feasible = [&](std::int64_t rides) {
        WGraph graph(6);
        for (int i = 0; i < 5; ++i) graph[i + 1].emplace_back(i, 0);
        graph[0].emplace_back(3, capacity[2]);
        graph[1].emplace_back(4, capacity[3]);
        graph[2].emplace_back(5, capacity[4]);
        graph[3].emplace_back(1, capacity[0] - rides);
        graph[4].emplace_back(2, capacity[1] - rides);
        graph[0].emplace_back(5, rides);
        graph[5].emplace_back(0, -rides);
        std::vector<std::int64_t> distances;
        return !SPFA(graph, distances, 0);
    };
    std::int64_t low = 0, high = total / 3 + 1;
    while (high - low > 1) {
        const auto middle = low + (high - low) / 2;
        if (feasible(middle)) low = middle;
        else high = middle;
    }
    verify_out << low << '\n';
}
