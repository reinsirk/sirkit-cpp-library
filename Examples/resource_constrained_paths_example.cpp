// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Resource_Constrained_Paths.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::ResourcePathArc> edges{
        {0, 1, 2, 0, 5},
        {0, 0, 0, 1, 3},
    };
    const sirkit::ResourceConstrainedPaths paths(2, edges, 0, 4, 1);
    assert(paths.minimum_time(1) == 8);
    assert(paths.minimum_time(1, 2) == 14);
    assert(!paths.minimum_time(1, 3));
}
