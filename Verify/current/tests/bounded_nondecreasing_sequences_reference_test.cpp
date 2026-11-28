// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Bounded_Nondecreasing_Sequences.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Limits = std::vector<std::uint32_t>;

std::uint32_t Naive(const Limits &lower, const Limits &upper)
{
    constexpr std::uint32_t kMod = 998244353;
    std::vector<std::uint32_t> count(*std::ranges::max_element(upper), 1);
    for (std::size_t i = 0; i < lower.size(); ++i)
    {
        std::uint64_t sum = 0;
        for (std::size_t value = 0; value < count.size(); ++value)
        {
            if (lower[i] <= value && value < upper[i])
            {
                sum += count[value];
            }
            count[value] = static_cast<std::uint32_t>(sum % kMod);
        }
    }
    return count.back();
}

void Compare(const Limits &lower, const Limits &upper)
{
    assert(sirkit::count_bounded_nondecreasing_sequences(lower, upper) == Naive(lower, upper));
}
}

int main()
{
    using sirkit::count_bounded_nondecreasing_sequences;
    assert(count_bounded_nondecreasing_sequences({}, {}) == 1);
    assert(count_bounded_nondecreasing_sequences(Limits{2, 0}, Limits{5, 2}) == 0);
    constexpr auto kMax = std::numeric_limits<std::uint32_t>::max();
    assert(count_bounded_nondecreasing_sequences(Limits{kMax - 4, kMax - 3}, Limits{kMax - 1, kMax}) == 8);

    try
    {
        static_cast<void>(count_bounded_nondecreasing_sequences({}, Limits{1}));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
    for (const auto n : {std::size_t{1}, (std::size_t{1} << 21) + 1})
    {
        try
        {
            static_cast<void>(count_bounded_nondecreasing_sequences(Limits(n, 0), Limits(n, 1U << 22)));
            assert(false);
        }
        catch (const std::length_error &)
        {
        }
    }

    Compare(Limits(109, 0), Limits(109, 130));
    Compare(Limits(15, 0), Limits(15, 129));
    Compare(Limits(149, 0), Limits(149, 13));
    for (const std::uint32_t n : {17u, 33u, 80u, 135u, 231u, 441u})
    {
        Limits lower(n), upper(n);
        for (std::uint32_t i = 0; i < n; ++i)
        {
            lower[i] = i / 3;
            upper[i] = i + 32;
        }
        Compare(lower, upper);
        for (std::uint32_t i = 0; i < n; ++i)
        {
            lower[i] = i;
            upper[i] = i + 1;
        }
        Compare(lower, upper);
    }
    std::mt19937 random(38125);
    for (int trial = 0; trial < 300; ++trial)
    {
        Limits lower(1 + random() % 125), upper(lower.size());
        for (std::size_t i = 0; i < lower.size(); ++i)
        {
            lower[i] = static_cast<std::uint32_t>(random() % 40);
            upper[i] = static_cast<std::uint32_t>(35 + random() % 30);
        }
        Compare(lower, upper);
    }
}
