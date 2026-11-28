// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Exact_Cover.cpp"

#include <array>
#include <cstddef>
#include <iostream>

int main()
{
    // 2マスを覆う。同じピース（列2）は高々1回使える。
    sirkit::ExactCover layout(2, 1);
    layout.add_row(std::array<std::size_t, 2>{0, 2});
    layout.add_row(std::array<std::size_t, 2>{1, 2});
    layout.add_row(std::array<std::size_t, 1>{1});
    if (const auto answer = layout.solve())
    {
        for (const auto row : *answer)
        {
            std::cout << row << '\n';
        }
    }
}
