// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_LIS_STATISTICS_DEFINED
#include "../../../Algorithm/LIS_Statistics.cpp"
#endif
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
template<bool Strict, class T>
void check(const std::vector<T> &values, std::uint64_t modulus)
{
    std::size_t longest = 0;
    std::uint64_t count = 0;
    std::size_t union_mask = 0, intersection = 0;
    for (std::size_t mask = 0; mask < (std::size_t{1} << values.size()); ++mask)
    {
        std::size_t length = 0;
        bool increasing = true;
        std::optional<T> previous;
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            if (((mask >> i) & 1U) == 0)
            {
                continue;
            }
            if (previous && (Strict ? !(*previous < values[i]) : values[i] < *previous))
            {
                increasing = false;
                break;
            }
            previous = values[i];
            ++length;
        }
        if (!increasing)
        {
            continue;
        }
        if (length > longest)
        {
            longest = length;
            count = 1;
            union_mask = intersection = mask;
        }
        else if (length == longest)
        {
            ++count;
            union_mask |= mask;
            intersection &= mask;
        }
    }
    const auto counted = sirkit::count_lis<Strict>(values, modulus);
    const auto membership = sirkit::lis_membership<Strict>(values);
    assert(counted.length == longest && counted.count == count % modulus && membership.size() == values.size());
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        const auto bit = std::size_t{1} << i;
        using State = sirkit::LISMembership;
        const auto expected = intersection & bit ? State::kAll : union_mask & bit ? State::kSome : State::kNone;
        assert(membership[i] == expected);
    }
}
} // namespace

int main()
{
    const auto maximum = std::numeric_limits<std::uint64_t>::max();
    check<true>(std::vector<int>{}, 1);
    check<false>(std::vector<int>{}, maximum);
    check<true>(std::vector<int>{1, 4, 3, 2, 5}, 2);
    check<false>(std::vector<int>{2, 2, 1, 1, 3, 3}, 1);
    check<true>(std::vector<bool>{true, false, false, true}, 17);
    check<false>(std::vector<bool>{true, false, false, true}, 17);
    const auto lo = std::numeric_limits<std::int64_t>::min();
    const auto hi = std::numeric_limits<std::int64_t>::max();
    check<true>(std::vector<std::int64_t>{hi, lo, 0, lo, hi}, maximum);
    check<false>(std::vector<std::int64_t>{hi, lo, 0, lo, hi}, maximum);

    std::mt19937 random(869243);
    const std::array<std::uint64_t, 5> moduli{1, 2, 17, 1'000'000'007, maximum};
    for (int trial = 0; trial < 300; ++trial)
    {
        std::vector<int> values(random() % 13);
        for (auto &value : values)
        {
            value = static_cast<int>(random() % 13) - 6;
        }
        const auto modulus = moduli[random() % moduli.size()];
        check<true>(values, modulus);
        check<false>(values, modulus);
    }

    // 2^64通りをUINT64_MAXで割った余りは1。
    std::vector<int> values(128);
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        values[i] = static_cast<int>(i / 2);
    }
    const auto wrapped = sirkit::count_lis(values, maximum);
    assert(wrapped.length == 64 && wrapped.count == 1);

    constexpr std::size_t kSize = 200'000;
    constexpr std::uint64_t kModulus = 1'000'000'007;
    values.resize(kSize);
    std::uint64_t power = 1;
    for (std::size_t i = 0; i < values.size(); i += 2)
    {
        values[i] = values[i + 1] = static_cast<int>(i / 2);
        power = power * 2 % kModulus;
    }
    const auto result = sirkit::count_lis(values);
    assert(result.length == kSize / 2 && result.count == power);
    for (const auto member : sirkit::lis_membership(values))
    {
        assert(member == sirkit::LISMembership::kSome);
    }

    bool threw = false;
    try
    {
        (void)sirkit::count_lis(values, 0);
    }
    catch (const std::invalid_argument &)
    {
        threw = true;
    }
    assert(threw);
}
