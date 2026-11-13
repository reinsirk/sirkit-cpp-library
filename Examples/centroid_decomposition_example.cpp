// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Centroid_Decomposition.cpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>
int main() {
    Graph graph{{1}, {0, 2}, {1, 3}, {2}};
    CentroidDecomposition decomposition(graph);
    std::cout << decomposition.root() << '\n'; // 1
    for (int p : decomposition.parents()) std::cout << p << ' ';
    std::cout << '\n'; // 1 -1 1 2
    std::cout << decomposition.parent(3) << ' ' << decomposition.depth(3) << '\n'; // 2 2
    std::cout << decomposition.component_size(decomposition.root()) << '\n'; // 4
    std::cout << decomposition.distance(0, 3) << '\n'; // 3
    const std::int64_t infinity = std::numeric_limits<std::int64_t>::max() / 4;
    std::vector<std::int64_t> best(graph.size(), infinity);
    auto mark = [&](int vertex) {
        for (auto a : decomposition.ancestors(vertex))
            best[static_cast<std::size_t>(a.vertex)] = std::min(best[static_cast<std::size_t>(a.vertex)], std::int64_t{a.distance});
    };
    auto nearest = [&](int vertex) {
        auto result = infinity;
        for (auto a : decomposition.ancestors(vertex))
            result = std::min(result, best[static_cast<std::size_t>(a.vertex)] + a.distance);
        return result;
    };
    mark(0); std::cout << nearest(3) << '\n'; // 3
    mark(2); std::cout << nearest(3) << '\n'; // 1
    std::cout << decomposition.order().size() << ' ' << decomposition.depths().size() << '\n'; // 4 4
    std::cout << decomposition.centroid_tree().size() << '\n'; // 4
    decomposition.build(graph, false);
    std::cout << decomposition.stores_distances() << '\n'; // 0
    auto copy = decomposition;
    auto moved = std::move(copy);
    std::cout << copy.empty() << ' ' << moved.size() << '\n'; // 1 4
    moved.clear(); std::cout << moved.root() << '\n'; // -1
}
