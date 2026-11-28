#include "../Graph/K_Shortest_Simple_Paths.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::ShortestPathEdge> edges{{0, 1, 2}, {1, 2, 3}, {0, 2, 6}};
    const auto routes = sirkit::k_shortest_simple_paths(3, edges, 0, 2, 4);
    assert(routes.size() == 2);
    assert(routes[0].distance == 5);
    assert(routes[1].distance == 6);
    assert(routes[0].vertices == std::vector({0, 1, 2}));
}
