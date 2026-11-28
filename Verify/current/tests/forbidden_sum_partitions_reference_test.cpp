// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Forbidden_Sum_Partitions.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

using Value = std::int64_t;
using Count = std::uint64_t;
using sirkit::count_partitions_avoiding_sums;

struct Mod11
{
    unsigned value{};

    explicit Mod11(unsigned val = 0) : value(val % 11)
    {
    }

    Mod11 &operator+=(Mod11 other)
    {
        value = (value + other.value) % 11;
        return *this;
    }

    Mod11 &operator-=(Mod11 other)
    {
        value = (value + 11 - other.value) % 11;
        return *this;
    }
};

Count brute(std::span<const Value> values, std::span<const Value> banned)
{
    if (values.empty())
    {
        return 1;
    }
    Count valid = 0;
    const auto options = std::size_t{1} << (values.size() - 1);
    for (std::size_t option = 0; option < options; ++option)
    {
        bool permitted = true;
        Value sum = 0;
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            sum += values[i];
            if (i + 1 == values.size() || ((option >> i) & 1) != 0)
            {
                if (std::ranges::find(banned, sum) != banned.end())
                {
                    permitted = false;
                    break;
                }
                sum = 0;
            }
        }
        valid += permitted;
    }
    return valid;
}

#ifndef SKIP_RUNTIME_CHECKS
void expect_overflow(std::span<const Value> values)
{
    bool caught = false;
    try
    {
        static_cast<void>(count_partitions_avoiding_sums<Count>(values, 0));
    }
    catch (const std::overflow_error &)
    {
        caught = true;
    }
    assert(caught);
}
#endif

void edge_cases()
{
    const std::span<const Value> empty;
    const std::array<Value, 5> zeros{};
    assert(count_partitions_avoiding_sums<Count>(empty, empty) == 1);
    assert(count_partitions_avoiding_sums<Count>(zeros, empty) == 16);
    assert(count_partitions_avoiding_sums<Count>(zeros, 0) == 0);

    constexpr Value low = std::numeric_limits<Value>::min(), high = std::numeric_limits<Value>::max();
    const std::array low_cell{low}, high_cell{high};
    assert(count_partitions_avoiding_sums<Count>(low_cell, high) == 1);
    assert(count_partitions_avoiding_sums<Count>(high_cell, low) == 1);
    assert(count_partitions_avoiding_sums<Count>(low_cell, low) == 0);
    assert(count_partitions_avoiding_sums<Count>(high_cell, high) == 0);

#ifndef SKIP_RUNTIME_CHECKS
    const std::array too_high{high, Value{1}}, too_low{low, Value{-1}};
    expect_overflow(too_high);
    expect_overflow(too_low);
#endif
}

void random_cases()
{
    std::mt19937 random(0xe04ae);
    for (int test = 0; test < 1800; ++test)
    {
        std::vector<Value> values(random() % 13), banned(random() % 8);
        for (auto &value : values)
        {
            value = static_cast<Value>(random() % 13) - 6;
        }
        for (auto &value : banned)
        {
            value = static_cast<Value>(random() % 11) - 5;
        }
        const auto expected = brute(values, banned);
        assert(count_partitions_avoiding_sums<Count>(values, banned) == expected);
        assert(count_partitions_avoiding_sums<Mod11>(values, banned).value == expected % 11);
    }
}

int main()
{
    edge_cases();
    random_cases();
}
