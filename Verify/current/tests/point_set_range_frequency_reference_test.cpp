// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Point_Set_Range_Frequency.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

int main()
{
    using Value = std::int64_t;
    using sirkit::PointSetRangeFrequency;
    constexpr auto kLow = std::numeric_limits<Value>::lowest();
    constexpr auto kHigh = std::numeric_limits<Value>::max();

    PointSetRangeFrequency<Value> empty(std::span<const Value>{});
    assert(empty.size() == 0);
    assert(empty.count(0, 0, kHigh) == 0);

    const std::vector<Value> initial{kLow, 4, 4, kHigh, 4};
    PointSetRangeFrequency<Value> frequency(initial);
    assert(frequency.size() == initial.size());
    assert(frequency.get(0) == kLow);
    assert(frequency.count(1, 4, 4) == 2);
    assert(frequency.count(2, 5, 4) == 2);
    assert(frequency.count(0, 5, kHigh) == 1);
    assert(frequency.count(0, 5, -3) == 0);
    frequency.set(1, 4);
    frequency.set(2, kLow);
    assert(frequency.get(2) == kLow);
    assert(frequency.count(0, 2, kLow) == 1);
    assert(frequency.count(0, 5, kLow) == 2);
    frequency.set(3, -3);
    assert(frequency.count(0, 5, kHigh) == 0);
    assert(frequency.count(0, 5, -3) == 1);

    auto copy = frequency;
    copy.set(2, kHigh);
    assert(frequency.get(2) == kLow);
    assert(copy.get(2) == kHigh);
    const auto moved = std::move(copy);
    assert(moved.count(2, 3, kHigh) == 1);

    std::mt19937_64 rng(5981143);
    for (int trial = 0; trial < 350; ++trial)
    {
        std::vector<Value> values(trial % 47 + 1);
        for (auto &value : values)
        {
            value = static_cast<Value>(rng() % 37) - 18;
        }
        PointSetRangeFrequency<Value> random_frequency(values);
        for (int step = 0; step < 220; ++step)
        {
            const auto position = rng() % values.size();
            auto value = static_cast<Value>(rng() % 59) - 29;
            if (step % 7 == 0)
            {
                value = step % 2 == 0 ? kHigh : kLow;
            }
            if (step % 3 != 0)
            {
                values[position] = value;
                random_frequency.set(position, value);
                assert(random_frequency.get(position) == value);
            }
            const auto [left, right] = std::minmax({rng() % (values.size() + 1), rng() % (values.size() + 1)});
            const auto expected = std::count(values.begin() + left, values.begin() + right, value);
            assert(random_frequency.count(left, right, value) == static_cast<std::size_t>(expected));
            assert(random_frequency.count(0, values.size(), values[position]) ==
                   static_cast<std::size_t>(std::count(values.begin(), values.end(), values[position])));
        }
    }

    sirkit::detail::CheckPointSetRangeFrequencySize(9, 9);
    try
    {
        sirkit::detail::CheckPointSetRangeFrequencySize(10, 9);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }

#ifndef SKIP_RUNTIME_CHECKS
    try
    {
        (void)empty.get(0);
        assert(false);
    }
    catch (const std::out_of_range &)
    {
    }
    try
    {
        frequency.set(5, 0);
        assert(false);
    }
    catch (const std::out_of_range &)
    {
    }
    for (const auto &[left, right] : {std::pair{2, 1}, std::pair{0, 6}})
    {
        try
        {
            (void)frequency.count(left, right, 0);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
