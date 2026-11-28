// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Subarray_Aggregate_Runs.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numeric>
#include <random>
#include <span>
#include <sstream>
#include <string>
#include <vector>

template <class T, class Combine, class Equal = std::equal_to<>>
void CheckRuns(const std::vector<T> &values, Combine combine, Equal equal = {})
{
    std::size_t processed = 0;
    sirkit::for_each_subarray_aggregate_run(
        values, combine,
        [&](std::size_t right, std::span<const sirkit::SubarrayAggregateRun<T>> runs)
        {
            assert(right == processed);
            ++processed;
            std::size_t expected_left = 0;
            for (std::size_t i = 0; i < runs.size(); ++i)
            {
                const auto &run = runs[i];
                assert(run.first == expected_left && run.first < run.last && run.last <= right + 1);
                if (i != 0)
                {
                    assert(!equal(run.value, runs[i - 1].value));
                }
                for (std::size_t left = run.first; left < run.last; ++left)
                {
                    auto expected = values[left];
                    for (auto j = left + 1; j <= right; ++j)
                    {
                        expected = combine(expected, values[j]);
                    }
                    assert(equal(expected, run.value));
                }
                expected_left = run.last;
            }
            assert(expected_left == right + 1);
        },
        equal);
    assert(processed == values.size());
}

int main()
{
    using U64 = std::uint64_t;
    constexpr U64 kMax = std::numeric_limits<U64>::max();
    CheckRuns(std::vector<U64>{}, std::gcd<U64, U64>);
    CheckRuns(std::vector<U64>{0, 0, kMax, kMax - 1, 0, 1, kMax}, std::gcd<U64, U64>);
    CheckRuns(std::vector<U64>{7, 7, 7}, std::gcd<U64, U64>);
    CheckRuns(std::vector<int>{1, 1, 1, 1, 1}, std::plus<>{});
    CheckRuns(std::vector<int>{5, 0, 2, 10, 3, 1}, std::plus<>{}, [](int a, int b)
              {
                  return a % 5 == b % 5;
              });
    CheckRuns(std::vector<std::string>{"ab", "", "cd", "ef", "", ""}, std::plus<>{});

    std::istringstream input("2 2 3 0");
    std::array<int, 4> minimum{-1, -1, -1, -1};
    sirkit::for_each_subarray_aggregate_run(
        std::ranges::istream_view<int>(input), [](int a, int b)
        {
            return std::min(a, b);
        },
        [&](std::size_t right, const auto &runs)
        {
            minimum[right] = runs.front().value;
        });
    assert((minimum == std::array<int, 4>{2, 2, 2, 0}));

    std::mt19937_64 rng(1036);
    for (int trial = 0; trial < 380; ++trial)
    {
        std::vector<U64> values(rng() % 49);
        for (auto &value : values)
        {
            value = rng() % 8192;
        }
        CheckRuns(values, std::gcd<U64, U64>);
        CheckRuns(values, std::bit_and<>{});
        CheckRuns(values, std::bit_or<>{});
    }
}
