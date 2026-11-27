// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Range_Linear_Add_Range_Min.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

int main()
{
    using sirkit::RangeLinearAddRangeMin;
    using I = std::int64_t;
    constexpr I kLow = std::numeric_limits<I>::min();
    constexpr I kHigh = std::numeric_limits<I>::max();
    constexpr I kLargeStep = I{1} << 62;

    RangeLinearAddRangeMin empty;
    assert(empty.size() == 0 && !empty.range_min(0, 0));
    empty.range_add(0, 0, kHigh, kHigh);

    // 累積傾きの絶対値が64ビットを超えても、両端の値はint64_tに収まる。
    const std::array<I, 4> positive_values{kHigh, kLow, 0, 0};
    RangeLinearAddRangeMin positive(positive_values);
    positive.range_add(0, 2, kHigh, -kHigh);
    positive.range_add(0, 2, kHigh, -kHigh);
    positive.range_add(0, 2, kHigh, 0);
    assert(positive.range_min(0, 2) == -kHigh);
    assert(positive.range_min(1, 2) == -1);
    positive.range_add(0, 1, 0, kHigh);
    assert(positive.range_min(0, 2) == -1);
    positive.range_add(0, 2, 0, 5);
    positive.range_add(1, 2, 0, 1);
    assert(positive.range_min(0, 2) == 5);

    const std::array<I, 4> negative_values{kLow, kHigh, kLow, kLow};
    RangeLinearAddRangeMin negative(negative_values);
    negative.range_add(0, 2, kLow, kHigh);
    negative.range_add(0, 2, kLow, kHigh);
    negative.range_add(0, 2, kLow, 0);
    assert(negative.range_min(0, 2) == -3);
    assert(negative.range_min(0, 1) == kHigh - 1);
    // b*i は64ビットの符号付き範囲外だが、加算後の値は表現できる。
    negative.range_add(2, 4, kLargeStep, 0);
    assert(negative.range_min(2, 4) == 0);
    assert(negative.range_min(3, 4) == kLargeStep);

    const std::array<I, 4> equal_cutoffs{kLow, kHigh, kLow, kHigh};
    RangeLinearAddRangeMin tied(equal_cutoffs);
    tied.range_add(0, 2, kLow, 0);
    tied.range_add(0, 2, -kHigh, 0);
    assert(tied.range_min(0, 1) == kLow && tied.range_min(1, 2) == kLow);
    assert(tied.range_min(0, 2) == kLow);

    std::mt19937_64 rng(6267137);
    for (int trial = 0; trial < 450; ++trial)
    {
        const int n = trial % 133 + 1;
        std::vector<I> values(n);
        for (auto &value : values)
        {
            value = static_cast<I>(rng() % 2001) - 1000;
        }
        RangeLinearAddRangeMin data(values);
        assert(data.size() == n);
        for (int i = 0; i < 240; ++i)
        {
            const auto [left, right] = std::minmax({static_cast<int>(rng() % (n + 1)), static_cast<int>(rng() % (n + 1))});
            if (i % 3 != 0)
            {
                const auto b = static_cast<I>(rng() % 20001) - 10000;
                const auto c = static_cast<I>(rng() % 200001) - 100000;
                data.range_add(left, right, b, c);
                for (int j = left; j < right; ++j)
                {
                    values[j] += b * j + c;
                }
            }
            if (left == right)
            {
                assert(!data.range_min(left, right));
            }
            else
            {
                assert(data.range_min(left, right) == *std::min_element(values.begin() + left, values.begin() + right));
            }
            assert(data.range_min(0, n) == *std::min_element(values.begin(), values.end()));
            const auto k = static_cast<int>(rng() % n);
            assert(data.range_min(k, k + 1) == values[k]);
        }
    }

    sirkit::detail::CheckRangeLinearAddRangeMinSize(std::numeric_limits<int>::max());
    try
    {
        sirkit::detail::CheckRangeLinearAddRangeMinSize(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }

#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[left, right] : {std::pair{-1, 4}, std::pair{3, 1}, std::pair{0, 5}})
    {
        try
        {
            (void)tied.range_min(left, right);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
