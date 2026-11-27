// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Assignment.cpp"
#include <cassert>

int main()
{
    const std::vector<std::vector<std::int64_t>> cost{{8, 1, 4}, {2, 5, 3}};
    const auto result = sirkit::minimum_assignment(cost);
    assert(result.cost == 3 && result.column[0] == 1 && result.column[1] == 0);
}
