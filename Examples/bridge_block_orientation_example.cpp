// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Bridge_Block_Orientation.cpp"

#include <cassert>
#include <utility>
#include <vector>

int main()
{
    const std::vector edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 0}, std::pair{2, 3}};
    const auto directed = sirkit::orient_bridge_blocks(4, edges);
    assert(directed == edges);
}
