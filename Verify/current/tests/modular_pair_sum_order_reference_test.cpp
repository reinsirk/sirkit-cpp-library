// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Modular_Pair_Sum_Order.cpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
void check(std::span<const std::uint64_t> left, std::span<const std::uint64_t> right, std::uint64_t modulus)
{
    const sirkit::ModularPairSumOrder actual(left, right, modulus);
    std::vector<std::uint64_t> expected;
    for (const auto a : left)
    {
        for (const auto b : right)
        {
            const auto first = a % modulus;
            const auto complement = modulus - b % modulus;
            expected.push_back(first >= complement ? first - complement : first + b % modulus);
        }
    }
    std::sort(expected.begin(), expected.end());
    assert(actual.size() == expected.size());
    assert(!actual.kth_smallest(0));
    assert(!actual.kth_smallest(actual.size() + 1));
    for (std::size_t index = 0; index < expected.size(); ++index)
    {
        assert(actual.kth_smallest(index + 1) == expected[index]);
    }
    for (const auto bound : expected)
    {
        const auto position = std::upper_bound(expected.begin(), expected.end(), bound);
        assert(actual.count_at_most(bound) == static_cast<std::uint64_t>(position - expected.begin()));
    }
    assert(actual.count_at_most(modulus) == expected.size());
}
} // namespace

int main()
{
    std::mt19937_64 rng(51301);
    for (std::size_t trial = 0; trial < 240; ++trial)
    {
        std::vector<std::uint64_t> left(trial % 13), right(trial % 17);
        for (auto &value : left)
        {
            value = rng() % 80;
        }
        for (auto &value : right)
        {
            value = rng() % 80;
        }
        check(left, right, 1 + rng() % 50);
    }
    constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
    const std::array<std::uint64_t, 5> extremes{0, 1, maximum - 1, maximum - 2, maximum};
    check(extremes, extremes, maximum);
    check(extremes, extremes, 1);

    const std::array<std::uint64_t, 1> left{7}, right{8};
    sirkit::ModularPairSumOrder original(left, right, 11);
    auto moved = std::move(original);
    assert(original.size() == 0 && !original.kth_smallest(1));
    assert(original.count_at_most(maximum) == 0);
    original = moved;
    assert(original.kth_smallest(1) == 4);
    moved = std::move(original);
    assert(moved.kth_smallest(1) == 4 && original.size() == 0);

    bool invalid_modulus = false;
    try
    {
        const sirkit::ModularPairSumOrder invalid(extremes, extremes, 0);
    }
    catch (const std::invalid_argument &)
    {
        invalid_modulus = true;
    }
    assert(invalid_modulus);
    bool too_many = false;
    try
    {
        sirkit::detail::check_modular_pair_sum_length(std::uint64_t{1} << 32);
    }
    catch (const std::length_error &)
    {
        too_many = true;
    }
    assert(too_many);
}
