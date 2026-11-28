// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Range_Add_Threshold_Min.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
using Value = std::int64_t;

std::optional<Value> brute(const std::vector<int> &keys, const std::vector<Value> &values,
                           std::size_t left, std::size_t right, int threshold)
{
    std::optional<Value> result;
    for (auto i = left; i < right; ++i)
    {
        if (keys[i] >= threshold && (!result || values[i] < *result))
        {
            result = values[i];
        }
    }
    return result;
}

template <class Exception, class Action>
void expect_exception(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}
}

int main()
{
    using Range = sirkit::RangeAddThresholdMin<int>;
    Range empty;
    assert(empty.empty() && empty.size() == 0);
    empty.add(0, 0, 5);
    assert(!empty.minimum_at_least(0));
    assert(Range({}, {}).empty());

    const std::array string_keys{std::string("a"), std::string("b"), std::string("a")};
    const std::array<Value, 3> values{10, 20, 5};
    sirkit::RangeAddThresholdMin<std::string> text_keys(string_keys, values, 10);
    assert(text_keys.minimum_at_least("a") == 5);
    assert(text_keys.minimum_at_least("b") == 20);
    assert(!text_keys.minimum_at_least("c"));
    auto copy = text_keys;
    copy.add(0, 1, -10);
    assert(copy.get(0) == 0 && text_keys.get(0) == 10);
    const auto moved = std::move(copy);
    assert(copy.empty() && moved.get(0) == 0);
    copy = moved;
    assert(copy.minimum_at_least("a") == 0);

    std::mt19937 gen(229148);
    for (int round = 0; round < 240; ++round)
    {
        const std::size_t n = gen() % 56 + 1;
        const std::size_t width = gen() % (2 * n + 1);
        std::vector<int> keys(n);
        std::vector<Value> actual(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            keys[i] = static_cast<int>(gen() % 9) - 4;
            actual[i] = static_cast<Value>(gen() % 101) - 50;
        }
        Range range(keys, actual, width);
        assert(range.size() == n && !range.empty());
        for (int step = 0; step < 150; ++step)
        {
            const auto first = gen() % (n + 1), second = gen() % (n + 1);
            const auto left = std::min(first, second), right = std::max(first, second);
            const auto delta = static_cast<Value>(gen() % 17) - 8;
            range.add(left, right, delta);
            for (auto i = left; i < right; ++i)
            {
                actual[i] += delta;
            }
            const auto threshold = static_cast<int>(gen() % 13) - 6;
            assert(range.minimum_at_least(left, right, threshold) == brute(keys, actual, left, right, threshold));
            assert(range.minimum_at_least(threshold) == brute(keys, actual, 0, n, threshold));
            const auto p = gen() % n;
            assert(range.get(p) == actual[p]);
        }
    }

    constexpr Value low = std::numeric_limits<Value>::min();
    constexpr Value high = std::numeric_limits<Value>::max();
    const std::array keys{0, 1, 2, 3};
    const std::array first{low, low + 1, low + 2, low + 3};
    Range positive(keys, first, 2);
    positive.add(0, 4, high);
    positive.add(0, 4, high - 2);
    assert(positive.get(0) == high - 3 && positive.get(3) == high);
    assert(positive.minimum_at_least(3) == high);
    positive.add(1, 3, -7);
    assert(positive.minimum_at_least(1) == high - 9);
    assert(positive.get(0) == high - 3 && positive.get(3) == high);
    positive.add(0, 4, low);
    assert(positive.get(0) == -4 && positive.get(3) == -1);

    const std::array pair_keys{0, 1};
    const std::array second{high - 1, high};
    Range negative(pair_keys, second, 2);
    negative.add(0, 2, -high);
    negative.add(0, 2, low + 1);
    assert(negative.get(0) == low && negative.minimum_at_least(1) == low + 1);
    negative.add(0, 2, 0);

    expect_exception<std::invalid_argument>([&]()
    {
        const Range mismatch(pair_keys, values);
    });
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::out_of_range>([&]() { static_cast<void>(positive.get(4)); });
    expect_exception<std::out_of_range>([&]() { positive.add(1, 5, 1); });
    expect_exception<std::out_of_range>([&]() { static_cast<void>(positive.minimum_at_least(2, 1, 0)); });

    const std::array<Value, 4> later_high{0, 1, 2, high};
    Range atomic_high(keys, later_high, 2);
    expect_exception<std::overflow_error>([&]() { atomic_high.add(0, 4, 1); });
    assert(atomic_high.get(0) == 0 && atomic_high.get(3) == high);

    const std::array<Value, 4> later_low{0, 1, low, 2};
    Range atomic_low(keys, later_low, 2);
    expect_exception<std::overflow_error>([&]() { atomic_low.add(1, 3, -1); });
    assert(atomic_low.get(1) == 1 && atomic_low.get(2) == low);
    expect_exception<std::overflow_error>([&]() { negative.add(0, 2, -1); });
    assert(negative.get(0) == low && negative.get(1) == low + 1);
#endif
}
