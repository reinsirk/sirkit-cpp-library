// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Incremental_APSP.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array edges{sirkit::IncrementalAPSPEdge{0, 1, 4}};
    sirkit::IncrementalAPSP paths(3, edges);
    assert(!paths.distance(0, 2));
    paths.add_edge(1, 2, 8);
    assert(paths.distance(0, 2) == 12);
    paths.add_edge(0, 1, 2);
    assert(paths.distance(0, 2) == 10);
    assert(!paths.distance(2, 0));
}
