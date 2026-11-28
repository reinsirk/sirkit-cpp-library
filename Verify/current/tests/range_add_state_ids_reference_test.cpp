// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Range_Add_State_Ids.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <random>
#include <stdexcept>
#include <vector>

template <class Error, class Operation>
static void check_error(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Error &)
    {
        caught = true;
    }
    assert(caught);
}

int main()
{
    using State = sirkit::RangeAddStateIds::state_type;
    sirkit::RangeAddStateIds empty;
    assert(empty.size() == 0 && empty.id() == 0 && empty.add(0, 0, 7) == 0);
    sirkit::RangeAddStateIds one(1);
    assert(one.add(0, 1, 0) == 0);
    constexpr auto kLow = std::numeric_limits<std::int64_t>::min();
    assert(one.add(0, 1, kLow) != 0);
    assert(one.add(0, 1, kLow) == 0);
    check_error<std::out_of_range>([&]
                                   {
                                       one.add(1, 0, 1);
                                   });
    check_error<std::out_of_range>([&]
                                   {
                                       one.add(0, 2, 1);
                                   });
    assert(sirkit::detail::checked_range_add_node_id(std::numeric_limits<State>::max()) == std::numeric_limits<State>::max());
    if constexpr (sizeof(std::size_t) > sizeof(State))
    {
        const auto excess = static_cast<std::size_t>(std::numeric_limits<State>::max()) + 1;
        check_error<std::length_error>([&]
                                       {
                                           (void)sirkit::detail::checked_range_add_node_id(excess);
                                       });
    }
    constexpr auto kHuge = std::numeric_limits<std::uint64_t>::max();
    sirkit::RangeAddStateIds wide(kHuge);
    const auto baseline = wide.add(0, kHuge, 1);
    wide.add(0, kHuge / 2, -1);
    assert(wide.add(kHuge / 2, kHuge, -1) == 0);
    assert(wide.add(0, kHuge, 1) == baseline);

    std::mt19937 random(797778);
    for (int test = 0; test < 180; ++test)
    {
        const auto n = 1 + random() % 51;
        sirkit::RangeAddStateIds states(n);
        std::vector<std::uint64_t> values(n);
        std::map<std::vector<std::uint64_t>, State> ids{{values, 0}};
        std::map<State, std::vector<std::uint64_t>> arrays{{0, values}};
        const auto check = [&](std::size_t left, std::size_t right, std::int64_t delta)
        {
            for (std::size_t i = left; i < right; ++i)
            {
                values[i] += static_cast<std::uint64_t>(delta);
            }
            const auto actual = states.add(left, right, delta);
            assert(ids.try_emplace(values, actual).first->second == actual);
            assert(arrays.try_emplace(actual, values).first->second == values);
            assert(states.id() == actual);
        };
        for (int step = 0; step < 120; ++step)
        {
            const auto left = random() % (n + 1), right = random() % (n + 1);
            const auto [first, last] = std::minmax(left, right);
            const auto delta = static_cast<int>(random() % 7) - 3;
            const auto previous = states.id();
            check(first, last, delta);
            if (step % 3 == 0)
            {
                check(first, last, -delta);
                assert(states.id() == previous);
            }
        }
    }
}
