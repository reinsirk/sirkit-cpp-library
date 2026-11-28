// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Round_Robin_Schedule.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

int main()
{
    bool rejected = false;
    try
    {
        static_cast<void>(sirkit::RoundRobinSchedule(-1));
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected);
    const sirkit::RoundRobinSchedule empty(0), single(1);
    assert(empty.size() == 0 && empty.round_count() == 0);
    assert(single.size() == 1 && single.round_count() == 0 && single.round_of(0, 0) == -1);
    const sirkit::RoundRobinSchedule eight(8);
    const std::array<std::pair<int, int>, 4> bad_queries{{{-1, 0}, {7, 0}, {0, -1}, {0, 8}}};
    for (const auto &[round, player] : bad_queries)
    {
        rejected = false;
        try
        {
            static_cast<void>(eight.partner(round, player));
        }
        catch (const std::out_of_range &)
        {
            rejected = true;
        }
        assert(rejected);
    }
    for (int n = 2; n <= 64; ++n)
    {
        const auto size = static_cast<std::size_t>(n);
        const sirkit::RoundRobinSchedule schedule(n);
        const auto pairs = n * (n - 1) / 2;
        const auto capacity_per_round = n / 2;
        assert(schedule.round_count() == (pairs + capacity_per_round - 1) / capacity_per_round);
        std::vector<unsigned> times(size * size);
        for (int round = 0; round < schedule.round_count(); ++round)
        {
            int games = 0;
            for (int vertex = 0; vertex < n; ++vertex)
            {
                const auto opponent = schedule.partner(round, vertex);
                if (opponent == -1)
                {
                    assert(n % 2 != 0 && vertex == round);
                    continue;
                }
                assert(opponent != vertex);
                assert(opponent >= 0 && opponent < n);
                assert(schedule.partner(round, opponent) == vertex);
                assert(schedule.round_of(vertex, opponent) == round);
                ++times[static_cast<std::size_t>(vertex) * size + static_cast<std::size_t>(opponent)];
                games += vertex < opponent;
            }
            assert(games == capacity_per_round);
        }
        for (std::size_t i = 0; i < size; ++i)
        {
            for (std::size_t j = 0; j < size; ++j)
            {
                assert(times[i * size + j] == (i == j ? 0U : 1U));
            }
        }
    }

    constexpr auto kMax = std::numeric_limits<int>::max();
    const sirkit::RoundRobinSchedule odd(kMax), even(kMax - 1);
    assert(odd.partner(kMax - 2, 0) == kMax - 4);
    assert(odd.round_of(kMax - 1, kMax - 2) == (kMax - 3) / 2);
    assert(even.round_of(kMax - 2, kMax - 3) == kMax - 3);
    assert(even.partner(kMax - 3, kMax - 2) == kMax - 3);
}
