// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

int main()
{
    std::size_t count{};
    std::array<std::size_t, 2> moves{};
    verify_in >> count >> moves[0] >> moves[1];
    std::vector<std::size_t> piles(count);
    for (auto &pile : piles)
    {
        verify_in >> pile;
    }
    const sirkit::SubtractionGame game(moves, *std::ranges::max_element(piles));
    verify_out << (game.winning_move(piles) ? "First\n" : "Second\n");
}
