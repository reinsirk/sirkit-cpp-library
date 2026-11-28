#include "../Graph/Two_Nearest_Source_Groups.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::ShortestPathEdge> edges{{0, 1, 2}, {1, 2, 3}};
    const std::vector<sirkit::GroupedShortestPathSource> sources{{0, 7}, {1, 7}, {2, 9}};
    const auto result = sirkit::two_nearest_source_groups(3, edges, sources, true);
    const auto &[nearest, next] = result[1];
    assert(nearest->source == 1 && nearest->group == 7);
    assert(next->distance == 3 && next->group == 9);
}
