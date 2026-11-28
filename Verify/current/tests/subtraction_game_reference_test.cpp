// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Subtraction_Game.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <map>
#include <random>
#include <set>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

static std::vector<std::size_t> reference(std::span<const std::size_t> moves, std::size_t limit)
{
    std::vector<std::size_t> grundies(limit + 1);
    for (std::size_t stones = 1; stones <= limit; ++stones)
    {
        std::set<std::size_t> reachable;
        for (auto move : moves)
        {
            if (move <= stones)
            {
                reachable.insert(grundies[stones - move]);
            }
        }
        while (reachable.contains(grundies[stones]))
        {
            ++grundies[stones];
        }
    }
    return grundies;
}

static bool minimax(std::span<const std::size_t> moves, std::vector<std::size_t> start)
{
    std::map<std::vector<std::size_t>, bool> known;
    const auto visit = [&](const auto &self, std::vector<std::size_t> piles) -> bool
    {
        std::ranges::sort(piles);
        if (const auto it = known.find(piles); it != known.end())
        {
            return it->second;
        }
        for (std::size_t pile = 0; pile < piles.size(); ++pile)
        {
            for (auto move : moves)
            {
                if (move <= piles[pile])
                {
                    auto next = piles;
                    next[pile] -= move;
                    if (!self(self, next))
                    {
                        return known.emplace(piles, true).first->second;
                    }
                }
            }
        }
        return known.emplace(piles, false).first->second;
    };
    return visit(visit, start);
}

int main()
{
    const sirkit::SubtractionGame empty({});
    assert(empty.grundy(0) == 0 && !empty.winning_move({}));
    const sirkit::SubtractionGame no_moves({}, 3);
    assert(no_moves.grundy(3) == 0);

    std::mt19937 rng(923131);
    for (int trial = 0; trial < 260; ++trial)
    {
        std::vector<std::size_t> moves(rng() % 10);
        for (auto &move : moves)
        {
            move = 1 + rng() % 16;
        }
        const auto expected = reference(moves, 73);
        sirkit::SubtractionGame game(moves, 3);
        for (std::size_t limit : {3, 15, 7, 40, 73, 68})
        {
            game.extend(limit);
            for (std::size_t i = 0; i <= limit; ++i)
            {
                assert(game.grundy(i) == expected[i]);
            }
        }
        std::vector<std::size_t> piles(rng() % 5);
        for (auto &stones : piles)
        {
            stones = rng() % 8;
        }
        const auto move = game.winning_move(piles);
        assert(move.has_value() == minimax(moves, piles));
        if (move)
        {
            assert(move->pile < piles.size() && move->removed <= piles[move->pile]);
            assert(std::ranges::find(moves, move->removed) != moves.end());
            piles[move->pile] -= move->removed;
            assert(!minimax(moves, piles));
        }
    }
    bool caught = false;
    try
    {
        (void)sirkit::SubtractionGame(std::array<std::size_t, 3>{5, 0, 7});
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught);
    caught = false;
    try
    {
        (void)empty.winning_move(std::array<std::size_t, 1>{1});
    }
    catch (const std::out_of_range &)
    {
        caught = true;
    }
    assert(caught);
    sirkit::SubtractionGame partial(std::array<std::size_t, 2>{2, 3}, 5);
    caught = false;
    try
    {
        partial.extend(std::numeric_limits<std::size_t>::max());
    }
    catch (const std::length_error &)
    {
        caught = true;
    }
    assert(caught && partial.grundy(5) == 0);
    auto transferred = std::move(partial);
    assert(transferred.grundy(5) == 0);
    partial.extend(3);
    assert(partial.grundy(0) == 0);
}
