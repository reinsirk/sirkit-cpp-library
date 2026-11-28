// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Knuth_Interval_DP.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

using sirkit::KnuthIntervalDP;
using Cost = std::int64_t;

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

void boundary_cases()
{
    const auto zero = [](int, int) -> Cost
    {
        return 0;
    };
    const auto unused = [](int, int) -> Cost
    {
        assert(false);
        return 0;
    };
    const KnuthIntervalDP no_items(0, unused), single(1, unused), no_length(5, 0, unused);
    assert(no_items.size() == 0 && no_items.max_length() == 0);
    assert(no_items.get(0, 0) == 0 && !no_items.split(0, 0));
    assert(single.get(0, 1) == 0 && !single.split(0, 1));
    assert(no_length.get(5, 5) == 0 && !no_length.split(5, 5));

    KnuthIntervalDP tied(5, zero);
    assert(tied.get(0, 5) == 0 && tied.split(0, 5) == 4);
    auto copied = tied;
    KnuthIntervalDP moved(std::move(copied));
    assert(moved.get(0, 5) == 0 && moved.split(0, 5) == 4);
    assert(copied.size() == 0 && copied.get(0, 0) == 0 && !copied.split(0, 0));
    const auto own_address = &moved;
    moved = std::move(*own_address);
    copied = std::move(moved);
    assert(moved.size() == 0 && copied.split(0, 5) == 4);
    moved = copied;
    assert(moved.split(0, 5) == 4);

    constexpr int maximum_size = std::numeric_limits<int>::max();
    expect_error<std::length_error>([&]
    {
        static_cast<void>(KnuthIntervalDP(maximum_size, zero));
    });

    constexpr auto maximum = std::numeric_limits<Cost>::max();
    constexpr auto minimum = std::numeric_limits<Cost>::min();
    const KnuthIntervalDP edge_max(2, [](int, int) { return maximum; });
    const KnuthIntervalDP edge_min(2, [](int, int) { return minimum; });
    assert(edge_max.get(0, 2) == maximum && edge_min.get(0, 2) == minimum);

#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::invalid_argument>([&] { static_cast<void>(KnuthIntervalDP(-1, 0, unused)); });
    expect_error<std::invalid_argument>([&] { static_cast<void>(KnuthIntervalDP(1, -1, unused)); });
    expect_error<std::invalid_argument>([&] { static_cast<void>(KnuthIntervalDP(1, 2, unused)); });
    expect_error<std::out_of_range>([&] { static_cast<void>(tied.get(-1, 0)); });
    expect_error<std::out_of_range>([&] { static_cast<void>(tied.get(2, 1)); });
    expect_error<std::out_of_range>([&] { static_cast<void>(tied.split(0, 6)); });
    expect_error<std::out_of_range>([&] { static_cast<void>(no_length.split(0, 1)); });
    expect_error<std::overflow_error>([]
    {
        static_cast<void>(KnuthIntervalDP(3, [](int, int) { return maximum; }));
    });
    expect_error<std::overflow_error>([]
    {
        static_cast<void>(KnuthIntervalDP(3, [](int, int) { return minimum; }));
    });
#endif
}

void randomized_costs()
{
    std::mt19937 random(0x5c631e);
    for (int attempt = 0; attempt < 950; ++attempt)
    {
        const int n = static_cast<int>(random() % 22);
        const int limit = static_cast<int>(random() % (n + 1));
        std::vector<Cost> prefix(static_cast<std::size_t>(n + 1));
        for (int i = 0; i < n; ++i)
        {
            prefix[i + 1] = prefix[i] + static_cast<Cost>(random() % 11);
        }
        const bool squared = attempt % 2 == 0;
        const auto cost = [&](int l, int r)
        {
            const auto sum = prefix[r] - prefix[l];
            return (squared ? sum * sum : sum) - 27;
        };
        const KnuthIntervalDP actual(n, limit, cost);
        assert(actual.size() == n && actual.max_length() == limit);

        std::vector<std::vector<Cost>> brute(static_cast<std::size_t>(n + 1),
                                            std::vector<Cost>(static_cast<std::size_t>(n + 1)));
        for (int span = 2; span <= limit; ++span)
        {
            for (int l = 0; l + span <= n; ++l)
            {
                const int r = l + span;
                Cost best = std::numeric_limits<Cost>::max();
                int split = l + 1;
                for (int k = l + 1; k < r; ++k)
                {
                    const Cost candidate = brute[l][k] + brute[k][r] + cost(l, r);
                    if (candidate <= best)
                    {
                        best = candidate;
                        split = k;
                    }
                }
                brute[l][r] = best;
                assert(actual.get(l, r) == best && actual.split(l, r) == split);
            }
        }
    }
}

int main()
{
    boundary_cases();
    randomized_costs();
}
