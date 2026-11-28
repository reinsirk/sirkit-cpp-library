// SPDX-License-Identifier: CC0-1.0
#include "../String/Grid_Pattern_Matching.cpp"

#include <array>
#include <cassert>
#include <string_view>

int main()
{
    const std::array<std::string_view, 3> text{"ababa", "babab", "ababa"};
    const std::array<std::string_view, 2> pattern{"aba", "bab"};
    const auto matches = sirkit::find_grid_pattern(text, pattern);
    const std::vector<std::pair<std::size_t, std::size_t>> expected{{0, 0}, {0, 2}, {1, 1}};
    assert(matches == expected);
}
