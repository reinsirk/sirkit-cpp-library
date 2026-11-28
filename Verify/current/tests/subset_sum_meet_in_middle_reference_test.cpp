// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_SUBSET_SUM_MEET_IN_MIDDLE_DEFINED
#include "../../../Algorithm/Subset_Sum_Meet_In_Middle.cpp"
#endif
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
void check(std::span<const std::int64_t> weights, std::span<const std::int64_t> targets)
{
    const sirkit::MeetInMiddleSubsetSum sums(weights);
    for (const auto target : targets)
    {
        std::optional<std::int64_t> max_sum;
        std::uint64_t at_most = 0;
        std::uint64_t exact = 0;
        for (std::uint64_t mask = 0; mask < (std::uint64_t{1} << weights.size()); ++mask)
        {
            std::int64_t sum = 0;
            for (std::size_t i = 0; i < weights.size(); ++i)
            {
                if ((mask >> i) & 1U)
                {
                    sum += weights[i];
                }
            }
            if (sum <= target)
            {
                ++at_most;
                if (!max_sum || sum > *max_sum)
                {
                    max_sum = sum;
                }
            }
            if (sum == target)
            {
                ++exact;
            }
        }
        assert(sums.count_at_most(target) == at_most);
        assert(sums.count_exact(target) == exact);
        const auto actual = sums.select_max_at_most(target);
        assert(actual.has_value() == max_sum.has_value());
        if (actual)
        {
            assert(actual->sum == *max_sum);
            assert((actual->mask >> weights.size()) == 0);
            std::int64_t witness_sum = 0;
            for (std::size_t i = 0; i < weights.size(); ++i)
            {
                if ((actual->mask >> i) & 1U)
                {
                    witness_sum += weights[i];
                }
            }
            assert(witness_sum == *max_sum);
        }
    }
}

template<class Exception>
void check_error(std::span<const std::int64_t> weights)
{
    bool thrown = false;
    try
    {
        (void)sirkit::MeetInMiddleSubsetSum(weights);
    }
    catch (const Exception &)
    {
        thrown = true;
    }
    assert(thrown);
}
} // namespace

int main()
{
    using Sum = std::int64_t;
    constexpr auto kMin = std::numeric_limits<Sum>::min();
    constexpr auto kMax = std::numeric_limits<Sum>::max();
    const std::array<Sum, 8> extremes{kMin, kMin + 1, -2, -1, 0, 1, kMax - 1, kMax};
    check({}, extremes);
    check(std::array<Sum, 2>{kMin, kMax}, extremes);
    check(std::array<Sum, 2>{kMax, kMin}, extremes);
    check(std::array<Sum, 5>{4, 4, 1, 1, 0}, extremes);
    check(std::array<Sum, 5>{-4, -4, -1, -1, 0}, extremes);
    check_error<std::overflow_error>(std::array<Sum, 3>{kMax, kMin, 1});
    check_error<std::overflow_error>(std::array<Sum, 3>{kMax, kMin, -1});
    check_error<std::length_error>(std::array<Sum, 45>{});

    std::mt19937 random(407151);
    for (int trial = 0; trial < 290; ++trial)
    {
        std::vector<Sum> weights(random() % 15);
        for (auto &weight : weights)
        {
            weight = static_cast<Sum>(random() % 101) - 50;
        }
        std::array<Sum, 8> targets;
        for (auto &target : targets)
        {
            target = static_cast<Sum>(random() % 501) - 250;
        }
        check(weights, targets);
    }

    std::array<Sum, 40> weights{};
    weights.back() = -1;
    auto sums = sirkit::MeetInMiddleSubsetSum(weights);
    auto copy = sums;
    sums = sirkit::MeetInMiddleSubsetSum(std::span<const Sum>{});
    assert(sums.count_exact(0) == 1);
    assert(copy.count_at_most(0) == (std::uint64_t{1} << 40));
    assert(copy.count_exact(-1) == (std::uint64_t{1} << 39));
    const auto selected = copy.select_max_at_most(-1);
    assert(selected && selected->sum == -1 && (selected->mask >> 39) == 1);
    assert(!copy.select_max_at_most(-2));
}
