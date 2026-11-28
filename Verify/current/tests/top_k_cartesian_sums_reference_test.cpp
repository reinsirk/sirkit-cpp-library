// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Top_K_Cartesian_Sums.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

using Groups = std::vector<std::vector<std::int64_t>>;
using Order = sirkit::CartesianSumOrder;

static void enumerate(const Groups &groups, std::size_t row, std::int64_t prefix, std::vector<std::int64_t> &all)
{
    if (row == groups.size())
    {
        all.push_back(prefix);
        return;
    }
    for (const auto weight : groups[row])
    {
        enumerate(groups, row + 1, prefix + weight, all);
    }
}

static void check(const Groups &groups, std::size_t k)
{
    std::vector<std::int64_t> all;
    enumerate(groups, 0, 0, all);
    const auto count = std::min(k, all.size());
    std::sort(all.begin(), all.end());
    auto min = std::vector<std::int64_t>(all.begin(), all.begin() + static_cast<std::ptrdiff_t>(count));
    auto max = std::vector<std::int64_t>(all.rbegin(), all.rbegin() + static_cast<std::ptrdiff_t>(count));
    assert(sirkit::top_k_cartesian_sums(groups, k, Order::smallest) == min);
    assert(sirkit::top_k_cartesian_sums(groups, k) == max);
}

static void check_overflow(const Groups &groups, Order order)
{
    bool caught = false;
    try
    {
        (void)sirkit::top_k_cartesian_sums(groups, 1, order);
    }
    catch (const std::overflow_error &)
    {
        caught = true;
    }
    assert(caught);
}

int main()
{
    check({}, 0);
    check({}, 1);
    check({{1, 2}, {}, {9}}, 7);
    check({{1, 2}, {7, 7, 7}, {9}}, 17);
    check({{2, -6, 0, 1, -100}, {-2, 3, -5, 10}, {-50, 2}}, 3);
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    // 途中の桁あふれは、後続の組で相殺できる。
    assert((sirkit::top_k_cartesian_sums(Groups{{kMax}, {kMax}, {kMin}}, 7) == std::vector<std::int64_t>{kMax - 1}));
    assert((sirkit::top_k_cartesian_sums(Groups{{kMin}, {kMin}, {kMax}, {kMax}}, 7) == std::vector<std::int64_t>{-2}));
    assert((sirkit::top_k_cartesian_sums(Groups{{kMax, kMin}, {1}}, 1, Order::smallest) == std::vector<std::int64_t>{kMin + 1}));
    assert((sirkit::top_k_cartesian_sums(Groups{{kMax, kMin}, {-1}}, 1) == std::vector<std::int64_t>{kMax - 1}));
    check_overflow({{kMax, kMin}, {1}}, Order::largest);
    check_overflow({{kMax, kMin}, {-1}}, Order::smallest);
    std::mt19937 rng(20261128);
    for (int t = 0; t < 500; ++t)
    {
        Groups groups(rng() % 6);
        for (auto &row : groups)
        {
            row.resize(rng() % 6);
            for (auto &entry : row)
            {
                entry = static_cast<int>(rng() % 21) - 10;
            }
        }
        check(groups, rng() % 35);
    }
    const Groups maximum(3, std::vector<std::int64_t>(1000, 10'000'000'000));
    assert(sirkit::top_k_cartesian_sums(maximum, 3000) == std::vector<std::int64_t>(3000, 30'000'000'000));
}
