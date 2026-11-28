#include "../Graph/Bottleneck_Pair_Histogram.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::BottleneckEdge> edges{{0, 1, 10}, {1, 2, 20}};
    const auto counts = sirkit::bottleneck_pair_histogram(3, edges);
    assert(counts.size() == 2);
    assert(counts[0].weight == 10 && counts[0].maximin_pairs == 2);
    assert(counts[1].weight == 20 && counts[1].minimax_pairs == 2);
}
