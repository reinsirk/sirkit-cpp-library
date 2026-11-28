// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<int> values(n);
    for (int &value : values)
    {
        verify_in >> value;
    }
    const int maximum = *std::ranges::max_element(values);
    sirkit::Graph moves(maximum + 1);
    for (int divisor = 1; divisor <= maximum / 2; ++divisor)
    {
        for (int multiple = 2 * divisor; multiple <= maximum; multiple += divisor)
        {
            moves[multiple].push_back(divisor);
        }
    }
    const auto grundy = sirkit::dag_grundy_numbers(moves);
    int combined = 0;
    for (const int value : values)
    {
        combined ^= (*grundy)[value];
    }
    verify_out << (combined == 0 ? "Bruno\n" : "Anna\n");
}
