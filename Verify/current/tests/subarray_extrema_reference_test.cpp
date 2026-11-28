// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Subarray_Extrema.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <random>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>

template <class Order>
static void check(const std::vector<int> &values, const Order &order)
{
    const std::span<const int> input(values);
    const auto spans = sirkit::subarray_extremum_spans(input, order);
    const auto sums = sirkit::subarray_extremum_sums_by_length<std::int64_t>(input, order);
    std::vector<std::int64_t> expected(values.size());
    std::vector<std::uint64_t> frequency(values.size());
    assert(spans.size() == values.size());
    for (std::size_t left = 0; left < values.size(); ++left)
    {
        std::size_t extremum = left;
        for (std::size_t right = left; right < values.size(); ++right)
        {
            if (order(values[right], values[extremum]))
            {
                extremum = right;
            }
            ++frequency[extremum];
            expected[right - left] += values[extremum];
            assert(spans[extremum].left <= left && right < spans[extremum].right);
        }
    }
    for (std::size_t i = 0; i < spans.size(); ++i)
    {
        assert(spans[i].left <= i && i < spans[i].right && spans[i].right <= values.size());
        assert((i - spans[i].left + 1) * (spans[i].right - i) == frequency[i]);
    }
    assert(sums == expected);
}

int main()
{
    check({}, std::less<>{});
    check({7}, std::less<>{});
    check({7, 7, 7, 7}, std::greater<>{});
    std::mt19937 rng(43929);
    for (int trial = 0; trial < 500; ++trial)
    {
        std::vector<int> values(rng() % 40);
        for (auto &value : values)
        {
            value = static_cast<int>(rng() % 38) - 19;
        }
        check(values, std::less<>{});
        check(values, std::greater<>{});
    }

    const auto compare_magnitude = [](int a, int b)
    {
        return a * a < b * b;
    };
    check({-7, 7, 1, -1}, compare_magnitude);
    const auto lazy = std::views::iota(std::size_t{1}, std::size_t{1000});
    const auto sums = sirkit::subarray_extremum_sums_by_length<std::uint64_t>(lazy);
    const auto maximum_sums = sirkit::subarray_extremum_sums_by_length<std::uint64_t>(lazy, std::greater<>{});
    assert(sums.size() == 999 && maximum_sums.size() == 999);
    for (std::uint64_t i = 0; i < 999; ++i)
    {
        assert(sums[i] == (999 - i) * (1000 - i) / 2);
        assert(maximum_sums[i] == (999 - i) * (1000 + i) / 2);
    }
    bool caught = false;
    try
    {
        const auto too_large = std::views::iota(std::size_t{0}, std::numeric_limits<std::size_t>::max());
        (void)sirkit::subarray_extremum_sums_by_length<std::uint64_t>(too_large);
    }
    catch (const std::length_error &)
    {
        caught = true;
    }
    assert(caught);
}
