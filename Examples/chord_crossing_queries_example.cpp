// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Chord_Crossing_Queries.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    using Chord = std::pair<std::int64_t, std::int64_t>;
    const std::array chords{Chord{2, 4}, Chord{6, 8}};
    const std::array queries{Chord{3, 7}, Chord{4, 7}};
    const auto counts = sirkit::count_chord_crossings(chords, queries);
    const std::vector<int> expected{2, 1};
    assert(counts == expected);
}
