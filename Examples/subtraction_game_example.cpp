// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Subtraction_Game.cpp"

#include <array>
#include <cstddef>
#include <iostream>

int main()
{
    const std::array<std::size_t, 2> moves{2, 3};
    const std::array<std::size_t, 2> piles{5, 8};
    const sirkit::SubtractionGame game(moves, 8);
    if (const auto move = game.winning_move(piles))
    {
        std::cout << move->pile << ' ' << move->removed << '\n'; // 1 2
    }
}
