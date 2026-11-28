#include "../Graph/Palindromic_Shortest_Paths.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::PalindromicPathEdge> edges{{0, 1, 'a'}, {1, 2, 'a'}};
    const sirkit::PalindromicShortestPaths paths(3, edges);
    assert(paths.distance(0, 2) == 2);
    const auto route = paths.path(0, 2);
    assert(route && (route->vertices == std::vector{0, 1, 2}));
    assert(!paths.path(2, 0));
}
