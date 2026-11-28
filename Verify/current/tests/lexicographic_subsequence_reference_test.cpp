// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Lexicographic_Subsequence.cpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
template <class Compare>
std::vector<std::size_t> brute(const std::vector<int> &values, std::size_t length, Compare less)
{
    std::vector<std::size_t> best;
    const auto projected_less = [&](std::size_t left, std::size_t right)
    {
        return less(values[left], values[right]);
    };
    for (std::uint32_t mask = 0; mask < (std::uint32_t{1} << values.size()); ++mask)
    {
        if (static_cast<std::size_t>(std::popcount(mask)) != length)
        {
            continue;
        }
        std::vector<std::size_t> candidate;
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            if (mask & (std::uint32_t{1} << i))
            {
                candidate.push_back(i);
            }
        }
        if (best.empty() || std::ranges::lexicographical_compare(candidate, best, projected_less)
            || (!std::ranges::lexicographical_compare(best, candidate, projected_less) && candidate < best))
        {
            best = candidate;
        }
    }
    return best;
}
}

int main()
{
    assert(sirkit::lexicographic_subsequence("", 0).empty());
    assert(sirkit::lexicographic_subsequence("abc", 3) == "abc");
    assert(sirkit::lexicographic_subsequence("atcoder", 3) == "acd");
    const std::string bytes{'\xff', '\x80', '\0', '\xff'};
    assert(sirkit::lexicographic_subsequence(bytes, 2) == std::string({'\0', '\xff'}));
    const std::vector<bool> bits{true, false, false, true, false};
    assert(sirkit::lexicographic_subsequence_indices(bits, 2) == (std::vector<std::size_t>{1, 2}));
    assert(sirkit::lexicographic_subsequence_indices(bits, 2, std::greater<>{}) == (std::vector<std::size_t>{0, 3}));
    const int same[] = {5, 5, 5, 5};
    assert(sirkit::lexicographic_subsequence_indices(same, 2) == (std::vector<std::size_t>{0, 1}));

    int combinations = 1;
    for (std::size_t n = 0; n <= 6; ++n)
    {
        for (int code = 0; code < combinations; ++code)
        {
            std::vector<int> values(n);
            auto digits = code;
            for (auto &value : values)
            {
                value = digits % 3;
                digits /= 3;
            }
            for (std::size_t length = 0; length <= n; ++length)
            {
                assert(sirkit::lexicographic_subsequence_indices(values, length) == brute(values, length, std::less<>{}));
                assert(sirkit::lexicographic_subsequence_indices(values, length, std::greater<>{})
                       == brute(values, length, std::greater<>{}));
            }
        }
        combinations *= 3;
    }

    std::mt19937 gen(449182);
    const auto buckets_less = [](int a, int b)
    {
        return a / 3 < b / 3;
    };
    for (int round = 0; round < 800; ++round)
    {
        std::vector<int> values(gen() % 10 + 1);
        for (auto &value : values)
        {
            value = static_cast<int>(gen() % 19);
        }
        const auto length = gen() % (values.size() + 1);
        assert(sirkit::lexicographic_subsequence_indices(values, length, buckets_less) == brute(values, length, buckets_less));
    }

#ifndef SKIP_RUNTIME_CHECKS
    bool rejected = false;
    try
    {
        static_cast<void>(sirkit::lexicographic_subsequence("a", 2));
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected);
#endif
}
