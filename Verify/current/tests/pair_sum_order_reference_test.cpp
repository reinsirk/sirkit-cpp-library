// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Pair_Sum_Order.cpp"

#include <array>
#include <bit>
#include <cassert>
#include <random>

using Value = std::int64_t;
using Sum = std::pair<int, Value>;
using Values = std::vector<Value>;

static Sum brute_sum(Value a, Value b)
{
    const auto bits = static_cast<std::uint64_t>(a) + static_cast<std::uint64_t>(b);
    const auto wrapped = std::bit_cast<Value>(bits);
    if (a < 0 && b < 0 && wrapped >= 0)
    {
        return {-1, wrapped};
    }
    if (a >= 0 && b >= 0 && wrapped < 0)
    {
        return {1, wrapped};
    }
    return {0, wrapped};
}

static void expect_rank(const sirkit::PairSumOrder &order, std::uint64_t rank, Sum expected)
{
    if (expected.first == 0)
    {
        assert(order.kth_smallest(rank) == expected.second);
        assert(order.kth_largest(order.size() - rank + 1) == expected.second);
        return;
    }
    bool small_failed = false;
    try
    {
        (void)order.kth_smallest(rank);
    }
    catch (const std::overflow_error &)
    {
        small_failed = true;
    }
    assert(small_failed);
    bool large_failed = false;
    try
    {
        (void)order.kth_largest(order.size() - rank + 1);
    }
    catch (const std::overflow_error &)
    {
        large_failed = true;
    }
    assert(large_failed);
}

static void expect_all(const sirkit::PairSumOrder &order, std::vector<Sum> sums)
{
    std::sort(sums.begin(), sums.end());
    assert(order.size() == sums.size());
    assert(!order.kth_smallest(0) && !order.kth_smallest(order.size() + 1));
    assert(!order.kth_largest(0) && !order.kth_largest(order.size() + 1));
    for (std::size_t rank = 1; rank <= sums.size(); ++rank)
    {
        expect_rank(order, rank, sums[rank - 1]);
    }

    constexpr auto kMin = std::numeric_limits<Value>::min();
    constexpr auto kMax = std::numeric_limits<Value>::max();
    for (const auto bound : {kMin, kMin + 1, Value{-20}, Value{-1}, Value{0}, Value{1}, Value{20}, kMax - 1, kMax})
    {
        const auto smaller = std::count_if(sums.begin(), sums.end(), [bound](Sum s) { return s < Sum{0, bound}; });
        const auto at_most = std::count_if(sums.begin(), sums.end(), [bound](Sum s) { return s <= Sum{0, bound}; });
        assert(order.count_below(bound) == static_cast<std::uint64_t>(smaller));
        assert(order.count_at_most(bound) == static_cast<std::uint64_t>(at_most));
        assert(order.count_equal(bound) == static_cast<std::uint64_t>(at_most - smaller));
    }
}

static void compare_both(const Values &left, const Values &right)
{
    std::vector<Sum> cross, within;
    for (const auto a : left)
    {
        for (const auto b : right)
        {
            cross.push_back(brute_sum(a, b));
        }
    }
    for (std::size_t i = 0; i < left.size(); ++i)
    {
        for (std::size_t j = i + 1; j < left.size(); ++j)
        {
            within.push_back(brute_sum(left[i], left[j]));
        }
    }
    expect_all(sirkit::PairSumOrder(left, right), cross);
    expect_all(sirkit::PairSumOrder(left), within);
}

int main()
{
    constexpr auto kMin = std::numeric_limits<Value>::min();
    constexpr auto kMax = std::numeric_limits<Value>::max();
    const Values left{3, kMax, 5, kMin, 3, -1, kMin};
    const Values right{4, 2, -4, kMax, kMin + 1, 0};
    compare_both(left, right);
    compare_both({}, {});
    compare_both(left, {});
    compare_both({kMin}, {kMin, 0, kMax});
    compare_both({kMin, kMin}, {kMin});
    compare_both({kMax, kMax}, {kMax});
    bool exceeds_count_limit = false;
    try
    {
        sirkit::detail::ValidatePairSumLength(std::uint64_t{1} << 32);
    }
    catch (const std::length_error &)
    {
        exceeds_count_limit = true;
    }
    assert(exceeds_count_limit);

    const Values half{5, 1, 3, 1};
    const Values sorted{1, 1, 3, 5};
    const sirkit::PairSumOrder single(half);
    assert(std::ranges::equal(single.sorted_left(), sorted));
    assert(std::ranges::equal(single.sorted_right(), sorted));
    assert(single.count_equal(4) == 2);
    assert(single.kth_largest(1) == 8);
    sirkit::PairSumOrder copy = single;
    const sirkit::PairSumOrder across(half, Values{6, 1});
    copy = across;
    assert(copy.size() == 8 && copy.count_equal(6) == 1);
    assert(std::ranges::equal(copy.sorted_right(), Values{1, 6}));
    auto local = std::move(copy);
    assert(copy.size() == 0 && copy.sorted_left().empty() && copy.sorted_right().empty());
    assert(local.kth_largest(1) == 11);
    copy = std::move(local);
    assert(local.size() == 0 && copy.size() == 8);
    auto &alias = copy;
    copy = std::move(alias);
    assert(copy.size() == 8 && copy.count_equal(6) == 1);

    constexpr std::array<Value, 11> pool{kMin, kMin + 1, -99, -4, -1, 0, 1, 4, 99, kMax - 1, kMax};
    std::mt19937 random(8461);
    for (int trial = 0; trial < 730; ++trial)
    {
        Values a(random() % 9), b(random() % 9);
        for (auto &v : a)
        {
            v = pool[random() % pool.size()];
        }
        for (auto &v : b)
        {
            v = pool[random() % pool.size()];
        }
        compare_both(a, b);
    }
}
