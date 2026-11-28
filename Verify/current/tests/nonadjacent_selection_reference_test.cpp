// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Nonadjacent_Selection.cpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <ranges>
#include <vector>

static void check_positions(const sirkit::NonadjacentSelection<> &profile, const std::vector<int> &values)
{
    for (std::size_t k = 0; k < profile.sums().size(); ++k)
    {
        const auto positions = profile.positions(k);
        assert(positions && positions->size() == k);
        std::int64_t sum = 0;
        for (std::size_t i = 0; i < k; ++i)
        {
            const auto index = (*positions)[i];
            assert(index < values.size());
            assert(i == 0 || (*positions)[i - 1] + 1 < index);
            sum += values[index];
        }
        assert(sum == profile.sums()[k]);
    }
    assert(!profile.positions(profile.sums().size()));
}

static void brute_check(const std::vector<int> &values, std::size_t limit)
{
    const auto maximum = std::min(limit, (values.size() + 1) / 2);
    std::vector<std::int64_t> expected(maximum + 1, std::numeric_limits<std::int64_t>::min());
    for (std::uint32_t mask = 0; mask < (std::uint32_t{1} << values.size()); ++mask)
    {
        const auto count = static_cast<std::size_t>(std::popcount(mask));
        if ((mask & (mask << 1)) != 0 || count > maximum)
        {
            continue;
        }
        std::int64_t sum = 0;
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            if ((mask >> i & 1) != 0)
            {
                sum += values[i];
            }
        }
        expected[count] = std::max(expected[count], sum);
    }
    const sirkit::NonadjacentSelection profile(values, limit);
    assert(profile.sums() == expected);
    check_positions(profile, values);
}

static void dp_check(const std::vector<int> &values)
{
    constexpr std::int64_t kNegInf = std::numeric_limits<std::int64_t>::min() / 4;
    const auto limit = (values.size() + 1) / 2;
    std::vector<std::int64_t> skip(limit + 1, kNegInf), take(limit + 1, kNegInf);
    skip[0] = 0;
    for (const auto weight : values)
    {
        for (auto count = limit; count > 0; --count)
        {
            skip[count] = std::max(skip[count], take[count]);
            take[count] = skip[count - 1] + weight;
        }
    }
    const sirkit::NonadjacentSelection profile(values);
    for (std::size_t i = 0; i <= limit; ++i)
    {
        assert(profile.sums()[i] == std::max(skip[i], take[i]));
    }
    check_positions(profile, values);
}

int main()
{
    brute_check({}, 100);
    brute_check({1, 2, 3}, 0);
    brute_check({-11}, 1);
    brute_check({-7, -7, -7, -7}, 7);
    brute_check({4, 8, 6}, 3);
    brute_check({5, 4, 0, -3, 7, 10, 9, 1, 2}, 9);
    std::mt19937 rng(312733);
    for (int trial = 0; trial < 450; ++trial)
    {
        const std::size_t size = rng() % 13;
        std::vector<int> values(size);
        for (auto &value : values)
        {
            value = static_cast<int>(rng() % 41) - 20;
        }
        brute_check(values, rng() % 10);
    }
    for (std::size_t n : {63, 120, 203})
    {
        std::vector<int> values(n);
        for (auto &value : values)
        {
            value = static_cast<int>(rng() % 21) - 10;
        }
        dp_check(values);
    }
    constexpr auto kLarge = std::numeric_limits<std::int64_t>::max();
    const sirkit::NonadjacentSelection single(std::vector{kLarge, kLarge, kLarge}, 1);
    assert(single.sums().back() == kLarge);
    const auto lazy = std::views::iota(1, 11);
    const sirkit::NonadjacentSelection<double> profile(lazy);
    assert(profile.sums().back() == 30.0 && profile.positions(5)->back() == 9);
}
