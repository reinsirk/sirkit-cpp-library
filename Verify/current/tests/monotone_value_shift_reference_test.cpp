// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Monotone_Value_Shift.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

using Value = std::int64_t;
using sirkit::MonotoneValueShift;

#ifndef SKIP_RUNTIME_CHECKS
template <typename Error, typename Operation>
void expect_error(Operation operation)
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
#endif

void boundary_cases()
{
    constexpr Value minimum = std::numeric_limits<Value>::min();
    constexpr Value maximum = std::numeric_limits<Value>::max();
    const MonotoneValueShift empty;
    assert(empty.empty() && empty.size() == 0 && empty.values().empty());
    assert(empty.lower_bound(minimum) == 0);
    assert(empty.lower_bound(maximum) == 0);
    assert(empty.upper_bound(maximum) == 0);

    MonotoneValueShift empty_span(std::span<const Value>{});
    empty_span.shift_between(minimum, maximum, maximum);
    assert(empty_span.count_between(minimum, maximum) == 0);

    const std::vector<Value> extremes{minimum, minimum, 0, maximum};
    MonotoneValueShift sequence(extremes);
    assert(!sequence.empty() && sequence.size() == 4);
    assert(sequence.lower_bound(minimum) == 0);
    assert(sequence.upper_bound(minimum) == 2);
    assert(sequence.lower_bound(maximum) == 3);
    assert(sequence.upper_bound(maximum) == 4);
    assert(sequence.count_between(minimum, maximum) == 4);
    sequence.shift_between(-1, -1, 7);
    sequence.shift_between(minimum, maximum, 0);
    assert(sequence.values() == extremes);

    // The first endpoint update temporarily wraps the aggregate unsigned key.
    sequence.shift_between(0, 0, 1);
    assert(sequence.get(2) == 1 && sequence.get(3) == maximum);
    sequence.shift_between(1, 1, -1);
    assert(sequence.values() == extremes);
    // The unsigned magnitude of INT64_MIN fits, though its signed magnitude does not.
    sequence.shift_between(0, 0, minimum);
    const std::vector<Value> collapsed{minimum, minimum, minimum, maximum};
    assert(sequence.values() == collapsed);
    sequence.shift_between(minimum, minimum, maximum);
    assert(sequence.get(0) == -1 && sequence.get(3) == maximum);
    sequence.shift_between(-1, -1, -1);
    assert(sequence.get(0) == -2 && sequence.get(3) == maximum);

    const std::vector<Value> only_maximum{maximum};
    MonotoneValueShift singleton(only_maximum);
    singleton.shift_between(maximum, maximum, -maximum);
    assert(singleton.get(0) == 0);
    singleton.shift_between(0, 0, maximum);
    assert(singleton.get(0) == maximum);

#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<Value> unsorted{2, 1};
    expect_error<std::invalid_argument>([&] { static_cast<void>(MonotoneValueShift(unsorted)); });
    expect_error<std::invalid_argument>([&] { static_cast<void>(sequence.count_between(1, 0)); });
    expect_error<std::invalid_argument>([&] { sequence.shift_between(1, 0, 0); });
    expect_error<std::out_of_range>([&] { static_cast<void>(sequence.get(-1)); });
    expect_error<std::out_of_range>([&] { static_cast<void>(sequence.get(sequence.size())); });
    expect_error<std::overflow_error>([&] { singleton.shift_between(maximum, maximum, 1); });
    const std::vector<Value> only_minimum{minimum};
    MonotoneValueShift lower_endpoint(only_minimum);
    expect_error<std::overflow_error>([&] { lower_endpoint.shift_between(minimum, minimum, -1); });
    assert(singleton.values() == only_maximum && lower_endpoint.values() == only_minimum);

    const std::vector<Value> consecutive{-1, 0, 1};
    MonotoneValueShift three(consecutive);
    expect_error<std::invalid_argument>([&] { three.shift_between(0, 0, 2); });
    assert(three.values() == consecutive);
    expect_error<std::invalid_argument>([&] { three.shift_between(0, 0, -2); });
    assert(three.values() == consecutive);
#endif
}

void randomized_updates()
{
    std::mt19937 random(0x2137fd);
    auto small = [&]()
    {
        return static_cast<Value>(random() % 41) - 20;
    };
    for (int test = 0; test < 250; ++test)
    {
        std::vector<Value> values(random() % 31);
        std::ranges::generate(values, small);
        std::ranges::sort(values);
        MonotoneValueShift sequence(values);
        for (int iteration = 0; iteration < 100; ++iteration)
        {
            const Value a = small();
            const Value b = small();
            const Value low = std::min(a, b);
            const Value high = std::max(a, b);
            const Value delta = static_cast<Value>(random() % 11) - 5;
            const auto left = std::ranges::lower_bound(values, low) - values.begin();
            const auto right = std::ranges::upper_bound(values, high) - values.begin();
            assert(sequence.lower_bound(low) == left);
            assert(sequence.upper_bound(high) == right);
            assert(sequence.count_between(low, high) == right - left);

            auto changed = values;
            for (auto &value : changed)
            {
                if (low <= value && value <= high)
                {
                    value += delta;
                }
            }
            if (std::ranges::is_sorted(changed))
            {
                sequence.shift_between(low, high, delta);
                values = std::move(changed);
            }
#ifndef SKIP_RUNTIME_CHECKS
            else
            {
                expect_error<std::invalid_argument>([&] { sequence.shift_between(low, high, delta); });
            }
#endif
            assert(sequence.values() == values);
            for (int i = 0; i < sequence.size(); ++i)
            {
                assert(sequence.get(i) == values[static_cast<std::size_t>(i)]);
            }
        }
    }
}

int main()
{
    boundary_cases();
    randomized_updates();
}
