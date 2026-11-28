#include "../Graph/Tree_Path_Bitwise_Counts.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::TreePathBitwiseEdge> edges{{0, 1, 1}, {0, 2, 3}};
    const auto counts = sirkit::tree_path_bitwise_counts(3, edges);
    assert(counts.and_pairs[0] == 3);
    assert(counts.or_pairs[1] == 2);
    assert(counts.xor_pairs[0] == 2);
}
