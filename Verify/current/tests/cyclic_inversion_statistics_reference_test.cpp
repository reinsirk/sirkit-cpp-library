// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Cyclic_Inversion_Statistics.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
template <class Range, class Compare>
void check(const sirkit::CyclicInversionStatistics &counts, const Range &values, Compare less)
{
    const auto n = values.size();
    std::vector<std::int64_t> expected(n);
    for (std::size_t start = 0; start < n; ++start)
    {
        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = i + 1; j < n; ++j)
            {
                expected[start] += less(values[(start + j) % n], values[(start + i) % n]);
            }
        }
    }
    assert(counts.size() == n && counts.empty() == values.empty());
    assert(counts.inversion_count() == (n == 0 ? 0 : expected[0]));
    assert(counts.left_rotation_counts() == expected);
    assert(counts.rotation_inversion_sum() == std::accumulate(expected.begin(), expected.end(), std::int64_t{0}));
}
}

int main()
{
    sirkit::CyclicInversionStatistics empty;
    check(empty, std::array<int, 0>{}, std::less<>{});
    check(sirkit::CyclicInversionStatistics(std::array<int, 0>{}), std::array<int, 0>{}, std::less<>{});
    check(sirkit::CyclicInversionStatistics(std::array{7}), std::array{7}, std::less<>{});
    const std::array ordered{10, 0, -6, -6, 4};
    sirkit::CyclicInversionStatistics reverse(ordered, std::greater<>{});
    check(reverse, ordered, std::greater<>{});
    const std::vector<bool> bits{true, false, true, false};
    check(sirkit::CyclicInversionStatistics(bits), bits, std::less<>{});

    const auto bucket_less = [](int a, int b)
    {
        return a / 3 < b / 3;
    };
    std::mt19937 gen(928331);
    for (int trial = 0; trial < 160; ++trial)
    {
        std::vector<int> numbers(gen() % 15 + 2);
        for (auto &value : numbers)
        {
            value = static_cast<int>(gen() % 17) - 8;
        }
        sirkit::CyclicInversionStatistics ordinary(numbers);
        sirkit::CyclicInversionStatistics buckets(numbers, bucket_less);
        for (int step = 0; step < 70; ++step)
        {
            const auto position = gen() % (numbers.size() - 1);
            ordinary.swap_adjacent(position);
            buckets.swap_adjacent(position);
            std::swap(numbers[position], numbers[position + 1]);
            check(ordinary, numbers, std::less<>{});
            check(buckets, numbers, bucket_less);
        }
    }

    auto copied = reverse;
    auto changed = ordered;
    std::swap(changed[0], changed[1]);
    copied.swap_adjacent(0);
    check(reverse, ordered, std::greater<>{});
    check(copied, changed, std::greater<>{});
    auto moved = std::move(copied);
    check(copied, std::array<int, 0>{}, std::less<>{});
    check(moved, changed, std::greater<>{});
    copied = moved;
    check(copied, changed, std::greater<>{});

    bool limit_rejected = false;
    try
    {
        sirkit::detail::check_cyclic_inversion_length(2'642'247);
    }
    catch (const std::length_error &)
    {
        limit_rejected = true;
    }
    assert(limit_rejected);
    sirkit::detail::check_cyclic_inversion_length(2'642'246);

#ifndef SKIP_RUNTIME_CHECKS
    for (const auto position : {std::size_t{4}, std::size_t{5}})
    {
        bool index_rejected = false;
        try
        {
            reverse.swap_adjacent(position);
        }
        catch (const std::out_of_range &)
        {
            index_rejected = true;
        }
        assert(index_rejected);
    }
    check(reverse, ordered, std::greater<>{});
#endif
}
