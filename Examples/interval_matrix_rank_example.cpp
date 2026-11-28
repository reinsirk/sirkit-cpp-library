// SPDX-License-Identifier: CC0-1.0
#include "../Math/Interval_Matrix_Rank.cpp"

#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    const std::vector<std::vector<std::uint32_t>> rows{{1, 0}, {2, 0}, {0, 1}};
    const auto ends = sirkit::interval_matrix_rank_ends(rows, 2);
    for (const auto end : ends)
    {
        if (end)
        {
            std::cout << end.value() << '\n';
        }
        else
        {
            std::cout << "-\n";
        }
    }
}
