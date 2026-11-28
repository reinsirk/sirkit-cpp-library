// SPDX-License-Identifier: CC0-1.0
#include "Algorithm/Line_Chip_Firing.cpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <map>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
sirkit::LineChipFiringResult brute(const std::vector<std::int64_t> &chips)
{
    std::map<std::int64_t, std::uint32_t> count;
    std::deque<std::int64_t> pending;
    for (const auto chip : chips)
    {
        if (++count[chip] == 2)
        {
            pending.push_back(chip);
        }
    }
    sirkit::LineChipFiringResult result{{}, 0};
    while (!pending.empty())
    {
        const auto at = pending.front();
        pending.pop_front();
        auto &here = count[at];
        const auto fires = here / 2;
        if (fires == 0)
        {
            continue;
        }
        here %= 2;
        result.operations += fires;
        for (const auto next : {at - 1, at + 1})
        {
            count[next] += fires;
            if (count[next] >= 2)
            {
                pending.push_back(next);
            }
        }
    }
    for (const auto &[position, multiplicity] : count)
    {
        if (multiplicity != 0)
        {
            result.positions.push_back(position);
        }
    }
    return result;
}

bool agrees(const std::vector<std::int64_t> &chips)
{
    const auto want = brute(chips);
    const auto got = sirkit::line_chip_firing(chips);
    return want.positions == got.positions && want.operations == got.operations;
}

bool check_sorted(std::vector<std::int64_t> &chips, std::size_t index, std::int64_t first)
{
    if (index == chips.size())
    {
        return agrees(chips);
    }
    for (auto position = first; position <= 3; ++position)
    {
        chips[index] = position;
        if (!check_sorted(chips, index + 1, position))
        {
            return false;
        }
    }
    return true;
}
}

int main()
{
    for (std::size_t n = 0; n <= 9; ++n)
    {
        std::vector<std::int64_t> chips(n);
        if (!check_sorted(chips, 0, -3))
        {
            return 1;
        }
    }
    std::mt19937 random(59353);
    for (std::size_t n = 10; n <= 40; ++n)
    {
        for (int repetition = 0; repetition < 20; ++repetition)
        {
            std::vector<std::int64_t> chips(n);
            for (auto &chip : chips)
            {
                chip = static_cast<int>(random() % 39) - 19 + 1000000000000;
            }
            if (!agrees(chips))
            {
                return 2;
            }
        }
    }
    constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
    constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
    if (!agrees({maximum - 1, minimum + 1, minimum + 1, maximum - 1, -1, -1}) ||
        !agrees({maximum, minimum}))
    {
        return 3;
    }
    for (const auto end : {minimum, maximum})
    {
        try
        {
            const std::vector<std::int64_t> chips{end, end};
            static_cast<void>(sirkit::line_chip_firing(chips));
            return 4;
        }
        catch (const std::overflow_error &)
        {
        }
    }
    try
    {
        const std::vector<std::int64_t> too_many((std::size_t{1} << 20) + 1);
        static_cast<void>(sirkit::line_chip_firing(too_many));
        return 5;
    }
    catch (const std::length_error &)
    {
    }
}
