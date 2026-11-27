// SPDX-License-Identifier: CC0-1.0
#include "../Graph/ST_Numbering.cpp"

#include <array>
#include <cassert>
#include <utility>
#include <vector>

int main()
{
    const std::array edges{std::pair{1, 2}, std::pair{2, 0}};
    const auto ranks = sirkit::st_numbering(3, 1, 0, edges);
    assert((ranks && *ranks == std::vector<int>{2, 0, 1}));
}
