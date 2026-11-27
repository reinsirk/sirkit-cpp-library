// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Three_Edge_Connected_Components.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{0, 1}, std::pair{0, 1}};
    const auto result = sirkit::three_edge_connected_component_ids(3, edges);
    assert(result.count == 2);
    assert(result.ids[0] == result.ids[1] && result.ids[0] != result.ids[2]);
}
