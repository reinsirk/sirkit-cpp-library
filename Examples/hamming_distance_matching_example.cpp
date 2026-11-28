// SPDX-License-Identifier: CC0-1.0
#include "../String/Hamming_Distance_Matching.cpp"
#include <cassert>
#include <cstddef>
#include <vector>

int main()
{
    const auto distances = sirkit::hamming_distances("aaba", "ab");
    const auto starts = sirkit::approximate_match_positions("aaba", "ab", 1);
    assert((distances == std::vector<std::size_t>{1, 0, 2}));
    assert((starts == std::vector<std::size_t>{0, 1}));
}
