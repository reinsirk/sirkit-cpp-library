// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Disjoint_Subarray_Max_Sum.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

static void check(const std::vector<std::int64_t> &values)
{
    const sirkit::DisjointSubarrayMaxSum data(values);
    const std::size_t n = values.size();
    assert(data.size() == n);
    assert(!data.max_sum(n + 1));
    assert(!data.intervals(n + 1));
    assert(data.max_sum(0) == 0);
    assert(data.intervals(0).value().empty());

    constexpr auto kImpossible = std::numeric_limits<std::int64_t>::min();
    std::vector<std::int64_t> previous(n + 1);
    for (std::size_t k = 1; k <= n; ++k)
    {
        std::vector<std::int64_t> best(n + 1, kImpossible);
        for (std::size_t r = 1; r <= n; ++r)
        {
            best[r] = best[r - 1];
            std::int64_t sum = 0;
            for (std::size_t l = r; l-- > 0;)
            {
                sum += values[l];
                if (previous[l] != kImpossible)
                {
                    best[r] = std::max(best[r], previous[l] + sum);
                }
            }
        }
        assert(data.max_sum(k) == best[n]);
        const auto intervals = data.intervals(k).value();
        assert(intervals.size() == k);
        std::size_t last = 0;
        std::int64_t total = 0;
        for (const auto &[l, r] : intervals)
        {
            assert(last <= l && l < r && r <= n);
            for (std::size_t i = l; i < r; ++i)
            {
                total += values[i];
            }
            last = r;
        }
        assert(total == best[n]);
        previous = std::move(best);
    }
}

int main()
{
    check({});
    check({80'000'000'000LL, -90'000'000'000LL, 75'000'000'000LL});
    int configurations = 1;
    for (int n = 1; n <= 7; ++n)
    {
        configurations *= 3;
        for (int mask = 0; mask < configurations; ++mask)
        {
            int state = mask;
            std::vector<std::int64_t> values(n);
            for (auto &value : values)
            {
                value = state % 3 - 1;
                state /= 3;
            }
            check(values);
        }
    }
    std::mt19937 random(17191);
    for (int trial = 0; trial < 400; ++trial)
    {
        std::vector<std::int64_t> values(random() % 29);
        for (auto &value : values)
        {
            value = static_cast<int>(random() % 101) - 50;
        }
        check(values);
    }
}
