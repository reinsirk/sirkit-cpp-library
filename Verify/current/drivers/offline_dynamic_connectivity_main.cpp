// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>
#include "Verify/current/common/fast_io.hpp"

int main() {
    int n{}, q{};
    if (!(verify_in >> n)) return 0;
    verify_in >> q;
    std::vector<std::int64_t> weights(static_cast<std::size_t>(n));
    for (auto& value : weights) verify_in >> value;
    OfflineDynamicConnectivity graph(weights);
    std::map<std::pair<int, int>, std::size_t> edges;
    for (int i = 0; i < q; ++i) {
        int type{}, u{};
        verify_in >> type; verify_in >> u;
        if (type < 2) {
            int v{}; verify_in >> v;
            const std::pair<int, int> endpoints = std::minmax(u, v);
            if (type == 0) edges[endpoints] = graph.add_edge(u, v);
            else { graph.remove_edge(edges.at(endpoints)); edges.erase(endpoints); }
        } else if (type == 2) {
            std::int64_t delta{}; verify_in >> delta; graph.add_vertex(u, delta);
        } else graph.query_component_sum(u);
    }
    for (const auto answer : graph.solve()) verify_out << answer << '\n';
}
