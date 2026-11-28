#include "../Graph/Equal_Tree_Path_Partition.cpp"

#include <cassert>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<int, int>> edges{{0, 1}, {1, 2}, {2, 3}};
    const auto paths = sirkit::partition_forest_into_equal_paths(4, edges, 2);
    assert(paths && paths->size() == 2);
    for (const auto &path : *paths)
    {
        assert(path.size() == 2);
    }
    assert(!sirkit::partition_forest_into_equal_paths(4, edges, 3));
}
