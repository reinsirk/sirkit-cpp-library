// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Chordal_Graph.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array path{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 3}};
    const auto chordal = sirkit::recognize_chordal_graph(4, path);
    assert(chordal.is_chordal && chordal.perfect_elimination_order.size() == 4);

    const std::array rim{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 3}, std::pair{3, 0}};
    const auto non_chordal = sirkit::recognize_chordal_graph(4, rim);
    assert(!non_chordal.is_chordal && non_chordal.induced_cycle.size() == 4);
}
