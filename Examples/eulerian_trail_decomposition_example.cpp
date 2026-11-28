#include "../Graph/Eulerian_Trail_Decomposition.cpp"

#include <cassert>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<int, int>> edges{{0, 1}, {0, 2}, {0, 3}};
    const auto trails = sirkit::minimum_eulerian_trail_decomposition<true>(4, edges);
    assert(trails.size() == 3);
    for (const auto &trail : trails)
    {
        assert(trail.vertices.front() == 0);
        assert(trail.edges.size() == 1);
    }
}
