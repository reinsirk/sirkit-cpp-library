// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Incremental_SCC.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 0}, std::pair{0, 1}};
    const auto events = sirkit::incremental_scc_merges(3, edges);
    assert(events.size() == 2);
    assert(events[0].time == 3 && events[1].time == 3);
}
