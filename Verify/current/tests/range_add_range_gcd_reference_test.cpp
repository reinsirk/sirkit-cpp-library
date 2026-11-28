// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Range_Add_Range_Gcd.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

template <class F>
static void expect_out_of_range(F operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const std::out_of_range &)
    {
        caught = true;
    }
    assert(caught);
}

int main()
{
    sirkit::RangeAddRangeGcd<> empty;
    assert(empty.size() == 0 && empty.gcd(0, 0) == 0);
    empty.add(0, 0, 7);
    expect_out_of_range([&]
                        {
                            (void)empty.get(0);
                        });

    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    const std::vector<std::int64_t> wide{kMin, kMax};
    sirkit::RangeAddRangeGcd extreme(wide);
    assert(extreme.get(0) == kMin && extreme.get(1) == kMax);
    assert(extreme.gcd(0, 1) == (std::uint64_t{1} << 63));
    assert(extreme.gcd(0, 2) == 1);
    extreme.add(0, 1, 1);
    assert(extreme.gcd(0, 2) == static_cast<std::uint64_t>(kMax));
    extreme.add(1, 2, kMin);
    assert(extreme.get(1) == -1 && extreme.gcd(0, 2) == 1);
    expect_out_of_range([&]
                        {
                            (void)extreme.get(-1);
                        });
    expect_out_of_range([&]
                        {
                            extreme.add(-1, 1, 0);
                        });
    expect_out_of_range([&]
                        {
                            (void)extreme.gcd(2, 1);
                        });
    expect_out_of_range([&]
                        {
                            (void)extreme.gcd(0, 3);
                        });

    const std::vector<std::int8_t> narrow{-128, 127};
    sirkit::RangeAddRangeGcd small(narrow);
    assert(small.gcd(0, 1) == 128);
    small.add(0, 1, 1);
    assert(small.gcd(0, 2) == 127);

    std::mt19937 random(931571);
    for (int trial = 0; trial < 440; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 65);
        std::vector<std::int64_t> values(n);
        for (auto &value : values)
        {
            value = static_cast<int>(random() % 4001) - 2000;
        }
        sirkit::RangeAddRangeGcd data(values);
        assert(data.size() == n);
        for (int step = 0; step < 150; ++step)
        {
            int l = static_cast<int>(random() % (n + 1));
            int r = static_cast<int>(random() % (n + 1));
            if (r < l)
            {
                std::swap(l, r);
            }
            const int delta = static_cast<int>(random() % 501) - 250;
            data.add(l, r, delta);
            for (int i = l; i < r; ++i)
            {
                values[i] += delta;
            }
            const int at = static_cast<int>(random() % n);
            assert(data.get(at) == values[at]);
            const auto expected = std::accumulate(values.begin() + l, values.begin() + r, std::int64_t{},
                                                  [](auto a, auto b)
                                                  {
                                                      return std::gcd(a, b);
                                                  });
            assert(data.gcd(l, r) == static_cast<std::uint64_t>(expected));
        }
    }
}
