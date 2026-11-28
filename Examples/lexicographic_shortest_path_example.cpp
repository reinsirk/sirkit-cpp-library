#include "../Graph/Lexicographic_Shortest_Path.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::ShortestPathEdge> edges{{0, 2, 1}, {2, 3, 1}, {0, 1, 1}, {1, 3, 1}};
    const auto route = sirkit::lexicographic_shortest_path(4, edges, 0, 3);
    assert(route && route->distance == 2);
    const std::vector expected{0, 1, 3};
    assert(route->vertices == expected);
}
