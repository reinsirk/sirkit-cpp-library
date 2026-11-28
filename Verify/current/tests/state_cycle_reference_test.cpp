// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/State_Cycle.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>

int main()
{
    for (int mapping = 0; mapping < 256; ++mapping)
    {
        std::array<int, 4> edges{};
        for (int i = 0; i < 4; ++i)
        {
            edges[i] = (mapping >> (2 * i)) & 3;
        }
        const auto next = [&](int state) { return edges[state]; };
        for (int start = 0; start < 4; ++start)
        {
            std::array first{-1, -1, -1, -1};
            int stop = start;
            int steps = 0;
            while (first[stop] < 0)
            {
                first[stop] = steps++;
                stop = next(stop);
            }
            const auto prefix = static_cast<std::uint64_t>(first[stop]);
            const auto period = static_cast<std::uint64_t>(steps - first[stop]);
            for (std::uint64_t budget = 0; budget <= 16; ++budget)
            {
                std::uint64_t used = 0;
                const auto bounded = [&](int state) {
                    ++used;
                    return next(state);
                };
                const auto cycle = sirkit::find_state_cycle(start, bounded, budget);
                assert(used <= budget);
                if (cycle)
                {
                    assert(cycle->prefix == prefix && cycle->period == period);
                }
                else
                {
                    assert(budget < 16);
                }
            }

            const auto project = [](int state) { return std::string(1, static_cast<char>('a' + state)); };
            std::string expected;
            int current = start;
            for (std::uint64_t count = 0; count <= 52; ++count)
            {
                assert(sirkit::fold_state_sequence(start, count, next, project, std::plus<>{}, std::string{}) == expected);
                expected += project(current);
                current = next(current);
            }
        }
    }

    const auto increasing = [](int state) { return state + 1; };
    assert(!sirkit::find_state_cycle(0, increasing, 100));
    assert(sirkit::fold_state_sequence(0, 100, increasing, std::identity{}, std::plus<>{}, 0) == 4950);

    struct State
    {
        std::uint64_t revision;
        int symbol;
    };
    const auto next = [](State state) { return State{state.revision + 1, (state.symbol + 1) % 3}; };
    const auto equal = [](State first, State second) { return first.symbol == second.symbol; };
    const auto cycle = sirkit::find_state_cycle(State{10, 0}, next, 50, equal);
    assert(cycle && cycle->prefix == 0 && cycle->period == 3);
    const auto project = [](State state) { return state.symbol; };
    assert(sirkit::fold_state_sequence(State{10, 0}, 10, next, project, std::plus<>{}, 0, equal) == 9);

    const auto fixed = [](std::uint64_t state) { return state; };
    const auto limit = std::numeric_limits<std::uint64_t>::max();
    assert(sirkit::fold_state_sequence(std::uint64_t{1}, limit, fixed, std::identity{}, std::plus<>{}, std::uint64_t{0}) == limit);
}
