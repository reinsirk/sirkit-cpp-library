// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Lexicographic_Subsequence.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <vector>

int main()
{
    assert(sirkit::lexicographic_subsequence("atcoder", 3) == "acd");
    const std::array values{3, 1, 1, 2};
    const std::vector<std::size_t> minimum{1, 2};
    const std::vector<std::size_t> maximum{0, 3};
    assert(sirkit::lexicographic_subsequence_indices(values, 2) == minimum);
    assert(sirkit::lexicographic_subsequence_indices(values, 2, std::greater<>{}) == maximum);
}
