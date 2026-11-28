// SPDX-License-Identifier: CC0-1.0
#include "DataStructure/Weighted_L1_Statistics.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
template <class Exception, class Function>
void expect_failure(Function function)
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
    using sirkit::WeightedL1Statistics;
    const std::array<std::int64_t, 7> input{5, -20, 0, 5, 17, -3, 5};
    constexpr std::array<std::int64_t, 5> keys{-20, -3, 0, 5, 17};
    WeightedL1Statistics statistics(input);
    assert(statistics.absolute_deviation(5) == 0 && !statistics.kth(0) && !statistics.minimizer_interval());
    expect_failure<std::out_of_range>([&]
                                      {
                                          statistics.insert(42);
                                      });
    expect_failure<std::out_of_range>([&]
                                      {
                                          (void)statistics.weight_at(1);
                                      });
    std::array<int, keys.size()> frequencies{};
    std::mt19937 rng(13274);
    for (int trial = 0; trial < 7000; ++trial)
    {
        const auto i = rng() % keys.size();
        const int count = static_cast<int>(rng() % 5);
        if (rng() % 2 == 0)
        {
            frequencies[i] += count;
            statistics.insert(keys[i], count);
        }
        else
        {
            const bool enough = count <= frequencies[i];
            assert(statistics.erase(keys[i], count) == enough);
            if (enough)
            {
                frequencies[i] -= count;
            }
        }
        std::vector<std::int64_t> sorted;
        for (std::size_t j = 0; j < keys.size(); ++j)
        {
            sorted.insert(sorted.end(), frequencies[j], keys[j]);
            assert(statistics.weight_at(keys[j]) == static_cast<std::uint64_t>(frequencies[j]));
        }
        assert(statistics.total_weight() == sorted.size());
        assert(!statistics.kth(sorted.size()));
        if (!sorted.empty())
        {
            auto rank = rng() % sorted.size();
            assert(statistics.kth(rank) == sorted[rank]);
            assert(statistics.minimizer_interval() == std::pair(sorted[(sorted.size() - 1) / 2], sorted[sorted.size() / 2]));
        }
        const int x = static_cast<int>(rng() % 101) - 50;
        std::uint64_t cost = 0;
        for (const auto key : sorted)
        {
            cost += static_cast<std::uint64_t>(std::max(key, std::int64_t{x}) - std::min(key, std::int64_t{x}));
        }
        assert(statistics.absolute_deviation(x) == cost);
    }

    constexpr auto kLo = std::numeric_limits<std::int64_t>::min();
    constexpr auto kHi = std::numeric_limits<std::int64_t>::max();
    constexpr auto kMaxWeight = std::numeric_limits<std::uint64_t>::max();
    WeightedL1Statistics extremes(std::array<std::int64_t, 2>{kLo, kHi});
    extremes.insert(kLo, kMaxWeight);
    assert(extremes.total_weight() == kMaxWeight && extremes.kth(kMaxWeight - 1) == kLo);
    expect_failure<std::overflow_error>([&]
                                        {
                                            extremes.insert(kHi);
                                        });
    expect_failure<std::overflow_error>([&]
                                        {
                                            (void)extremes.absolute_deviation(kHi);
                                        });
    assert(extremes.erase(kLo, kMaxWeight - 1));
    assert(extremes.absolute_deviation(kHi) == kMaxWeight);
    extremes.insert(kHi);
    assert(extremes.minimizer_interval() == std::pair(kLo, kHi));
    assert(extremes.absolute_deviation(0) == kMaxWeight);
    extremes.insert(kHi);
    expect_failure<std::overflow_error>([&]
                                        {
                                            (void)extremes.absolute_deviation(kLo);
                                        });
    auto copy = extremes;
    assert(copy.erase(kHi, 2));
    assert(extremes.total_weight() == 3 && copy.total_weight() == 1);
    auto only_low = std::move(copy);
    assert(copy.total_weight() == 0 && only_low.total_weight() == 1);
    copy = only_low;
    assert(copy.weight_at(kLo) == 1);
    extremes = std::move(copy);
    assert(copy.total_weight() == 0 && extremes.total_weight() == 1);
}
