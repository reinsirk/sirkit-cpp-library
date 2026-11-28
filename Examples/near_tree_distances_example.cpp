// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Near_Tree_Distances.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    using Edge = std::pair<int, int>;
    const std::array edges{Edge{0, 1}, Edge{1, 2}, Edge{2, 3}, Edge{3, 4}, Edge{4, 0}};
    const sirkit::NearTreeDistances distance(6, edges);
    assert(distance.distance(1, 4) == 2);
    assert(distance.distance(1, 5) == -1);
}
