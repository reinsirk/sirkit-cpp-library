// SPDX-License-Identifier: CC0-1.0
#include "DataStructure/Weighted_Slope_Trick.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>

namespace
{
template <class Exception, class Function>
void expect_throw(Function function)
{
    try
    {
        function();
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
}

int main()
{
    using sirkit::WeightedSlopeTrick;
    constexpr int kRadius = 256;
    std::mt19937 rng(187551);
    for (int round = 0; round < 300; ++round)
    {
        std::array<std::int64_t, 2 * kRadius + 1> values{};
        WeightedSlopeTrick model;
        for (int step = 0; step < 24; ++step)
        {
            const int center = static_cast<int>(rng() % 31) - 15;
            const int weight = static_cast<int>(rng() % 20);
            const int type = static_cast<int>(rng() % 8);
            if (type <= 2)
            {
                if (type == 0)
                {
                    model.add_left_hinge(center, weight);
                }
                else if (type == 1)
                {
                    model.add_right_hinge(center, weight);
                }
                else
                {
                    model.add_abs(center, weight);
                }
                for (int x = -kRadius; x <= kRadius; ++x)
                {
                    const int rise = type == 0   ? std::max(center - x, 0)
                                     : type == 1 ? std::max(x - center, 0)
                                                 : std::abs(x - center);
                    values[x + kRadius] += static_cast<std::int64_t>(weight) * rise;
                }
            }
            else if (type == 3)
            {
                model.add_constant(center);
                for (auto &v : values)
                {
                    v += center;
                }
            }
            else if (type == 4 || type == 5)
            {
                const int lo = static_cast<int>(rng() % 5) - 2;
                const int hi = type == 4 ? lo : lo + static_cast<int>(rng() % 5);
                if (type == 4)
                {
                    model.shift(lo);
                }
                else
                {
                    model.shift(lo, hi);
                }
                const auto old = values;
                for (int j = 0; j <= 2 * kRadius; ++j)
                {
                    const int first = std::clamp(j - hi, 0, 2 * kRadius);
                    const int last = std::clamp(j - lo, 0, 2 * kRadius);
                    values[j] = *std::min_element(old.begin() + first, old.begin() + last + 1);
                }
            }
            else if (type == 6)
            {
                model.clear_left();
                for (int j = 2 * kRadius - 1; j >= 0; --j)
                {
                    values[j] = std::min(values[j], values[j + 1]);
                }
            }
            else
            {
                model.clear_right();
                for (int j = 1; j <= 2 * kRadius; ++j)
                {
                    values[j] = std::min(values[j], values[j - 1]);
                }
            }
            const auto smallest = *std::min_element(values.begin(), values.end());
            assert(model.min_value() == smallest);
            const auto best = model.argmin();
            for (int x = -30; x <= 30; ++x)
            {
                assert(model.eval(x) == values[x + kRadius]);
                assert(((!best.lower || x >= *best.lower) && (!best.upper || x <= *best.upper)) ==
                       (model.eval(x) == smallest));
            }
        }
    }

    constexpr auto kLo = std::numeric_limits<std::int64_t>::min();
    constexpr auto kHi = std::numeric_limits<std::int64_t>::max();
    WeightedSlopeTrick large;
    large.add_constant(kLo);
    expect_throw<std::overflow_error>([&]
                                      {
                                          large.add_constant(-1);
                                      });
    large.add_left_hinge(kHi);
    assert(large.eval(kLo) == kHi);
    large.add_right_hinge(kLo);
    assert(large.min_value() == kHi);
    assert(large.argmin().lower == kLo && large.argmin().upper == kHi);
    expect_throw<std::overflow_error>([&]
                                      {
                                          large.add_constant(1);
                                      });
    expect_throw<std::overflow_error>([&]
                                      {
                                          large.shift(1);
                                      });
    assert(large.argmin().lower == kLo && large.argmin().upper == kHi);
    expect_throw<std::overflow_error>([&]
                                      {
                                          large.shift(-1);
                                      });
    expect_throw<std::invalid_argument>([&]
                                        {
                                            large.shift(1, -1);
                                        });

    WeightedSlopeTrick overflow;
    overflow.add_left_hinge(kHi, 2);
    expect_throw<std::overflow_error>([&]
                                      {
                                          overflow.add_right_hinge(kLo);
                                      });
    expect_throw<std::overflow_error>([&]
                                      {
                                          (void)overflow.eval(kLo);
                                      });
    assert(overflow.argmin().lower == kHi && !overflow.argmin().upper);
    WeightedSlopeTrick key_low;
    key_low.add_abs(kLo);
    key_low.shift(1);
    expect_throw<std::overflow_error>([&]
                                      {
                                          key_low.add_abs(kLo);
                                      });
    assert(key_low.argmin().lower == kLo + 1 && key_low.min_value() == 0);
    WeightedSlopeTrick key_high;
    key_high.add_abs(kHi);
    key_high.shift(-1);
    expect_throw<std::overflow_error>([&]
                                      {
                                          key_high.add_abs(kHi);
                                      });
    assert(key_high.argmin().upper == kHi - 1 && key_high.min_value() == 0);

    WeightedSlopeTrick rolling_back;
    for (int x = 0; x < 12; ++x)
    {
        rolling_back.add_left_hinge(x);
    }
    rolling_back.add_constant(kHi - 55);
    expect_throw<std::overflow_error>([&]
                                      {
                                          rolling_back.add_right_hinge(0, 12);
                                      });
    assert(rolling_back.argmin().lower == 11 && !rolling_back.argmin().upper);
    assert(rolling_back.eval(10) == kHi - 54 && rolling_back.eval(20) == kHi - 55);
    WeightedSlopeTrick outside;
    outside.add_left_hinge(kLo);
    outside.add_left_hinge(0);
    expect_throw<std::overflow_error>([&]
                                      {
                                          outside.shift(-1);
                                      });
    assert(outside.argmin().lower == 0);

    WeightedSlopeTrick flags;
    flags.add_abs(2, 10);
    auto duplicate = flags;
    duplicate.add_left_hinge(3, 1);
    assert(flags.eval(2) == 0 && duplicate.eval(2) == 1);
    auto moved = std::move(duplicate);
    assert(duplicate.eval(2) == 0 && moved.eval(2) == 1);
    moved = flags;
    assert(moved.eval(2) == 0);
    duplicate = std::move(flags);
    assert(flags.eval(4) == 0 && duplicate.eval(4) == 20);
}
