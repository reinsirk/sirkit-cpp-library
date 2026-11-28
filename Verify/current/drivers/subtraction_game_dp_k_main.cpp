// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    std::size_t count{}, stones{};
    verify_in >> count >> stones;
    std::vector<std::size_t> moves(count);
    for (auto &move : moves)
    {
        verify_in >> move;
    }
    const sirkit::SubtractionGame game(moves, stones);
    verify_out << (game.grundy(stones) == 0 ? "Second\n" : "First\n");
}
