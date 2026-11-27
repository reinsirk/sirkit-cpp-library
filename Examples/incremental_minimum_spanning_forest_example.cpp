// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Incremental_Minimum_Spanning_Forest.cpp"

#include <cassert>

int main()
{
    sirkit::IncrementalMinimumSpanningForest forest(3);
    assert(forest.add_edge(0, 1, 5).selected);
    assert(forest.add_edge(1, 2, 8).selected);
    const auto update = forest.add_edge(0, 2, 1);
    assert(update.edge_id == 2 && update.selected && update.removed_edge == 1);
    assert(forest.total_weight() == 6);
}
