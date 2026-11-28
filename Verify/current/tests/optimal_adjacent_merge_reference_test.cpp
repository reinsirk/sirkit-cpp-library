// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Optimal_Adjacent_Merge.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

static std::uint64_t brute(const std::vector<std::uint64_t> &weights)
{
    const auto n = weights.size();
    std::vector dp(n + 1, std::vector<std::uint64_t>(n + 1));
    for (std::size_t right = 1; right <= n; ++right)
    {
        auto sum = weights[right - 1];
        for (std::size_t left = right - 1; left != 0;)
        {
            --left;
            sum += weights[left];
            auto best = std::numeric_limits<std::uint64_t>::max();
            for (std::size_t mid = left + 1; mid < right; ++mid)
            {
                best = std::min(best, dp[left][mid] + dp[mid][right]);
            }
            dp[left][right] = best + sum;
        }
    }
    return dp[0][n];
}

static void check(const std::vector<std::uint64_t> &weights, std::uint64_t expected)
{
    const auto answer = sirkit::optimal_adjacent_merge(weights);
    assert(answer.cost == expected);
    const auto n = weights.size();
    if (n < 2)
    {
        assert(answer.merges.empty());
        return;
    }
    assert(answer.merges.size() == n - 1);
    struct Block
    {
        std::size_t begin = 0;
        std::size_t end = 0;
        std::uint64_t weight = 0;
        bool live = false;
    };
    std::vector<Block> blocks(n + answer.merges.size());
    for (std::size_t i = 0; i < n; ++i)
    {
        blocks[i] = {i, i + 1, weights[i], true};
    }
    std::uint64_t cost = 0;
    for (std::size_t i = 0; i < answer.merges.size(); ++i)
    {
        const auto [left, right] = answer.merges[i];
        assert(left < n + i && right < n + i);
        auto &a = blocks[left];
        auto &b = blocks[right];
        assert(a.live && b.live && a.end == b.begin);
        a.live = b.live = false;
        blocks[n + i] = {a.begin, b.end, a.weight + b.weight, true};
        cost += blocks[n + i].weight;
    }
    assert(blocks.back().begin == 0 && blocks.back().end == n);
    assert(cost == expected);
}

static void check_overflow(const std::vector<std::uint64_t> &weights)
{
    bool thrown = false;
    try
    {
        (void)sirkit::optimal_adjacent_merge(weights);
    }
    catch (const std::overflow_error &)
    {
        thrown = true;
    }
    assert(thrown);
}

int main()
{
    using Aggregate = sirkit::detail::AdjacentMergeAggregate;
    const auto item = Aggregate::Singleton(5, 7);
    const auto identity = Aggregate::NoAction();
    const auto mapped = Aggregate::Map(Aggregate::Compose(identity, identity), item);
    assert(mapped.count == 1 && mapped.maximum == 5 && mapped.first[0].index == 7);

    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    check({}, 0);
    check({kMax}, 0);
    check({0, kMax}, kMax);
    check({7, 6, 8, 6, 1, 1}, 68);
    check({0, 0, 4, 0, 0}, 8);
    check_overflow({kMax, 1});
    check_overflow({kMax / 2, kMax / 2, 1});

    std::mt19937 generator(273491);
    for (int n = 2; n <= 30; ++n)
    {
        for (int trial = 0; trial < 48; ++trial)
        {
            std::vector<std::uint64_t> weights(n);
            for (auto &weight : weights)
            {
                weight = generator() % ((trial % 2 == 0) ? 4 : 400);
            }
            check(weights, brute(weights));
        }
    }
    constexpr int kPower = 15;
    const auto n = std::size_t{1} << kPower;
    check(std::vector<std::uint64_t>(n, 3), 3 * n * kPower);
    std::vector<std::uint64_t> sparse(n);
    sparse[n / 2] = kMax / 2;
    check(sparse, kMax - 1);
}
