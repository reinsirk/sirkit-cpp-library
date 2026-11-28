// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Dominance_Counts.cpp"

#include <cassert>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<int, int>> points{{1, 2}, {1, 2}, {2, 3}, {3, 1}};
    const auto counts = sirkit::dominance_counts(points);
    const std::vector<int> expected{2, 2, 3, 1};
    assert(counts == expected);

    const auto strict = sirkit::dominance_counts(points, true, true);
    const std::vector<int> expected_strict{0, 0, 2, 0};
    assert(strict == expected_strict);
}
