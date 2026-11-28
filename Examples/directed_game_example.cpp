// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Directed_Game.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{2, 2}};
    const auto positions = sirkit::solve_directed_game(3, edges);
    using Outcome = sirkit::DirectedGameOutcome;
    assert(positions[0].outcome == Outcome::win && positions[0].next == 1);
    assert(positions[1].outcome == Outcome::lose && positions[1].plies == 0);
    assert(positions[2].outcome == Outcome::draw && positions[2].next == 2);
}
