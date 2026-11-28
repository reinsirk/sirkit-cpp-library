// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Bounded_Zigzag_Subsequence.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
using Integer = std::int64_t;
using Unsigned = std::uint64_t;

template <class Exception, class Action>
void expect_exception(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

bool valid(std::span<const Integer> values, std::span<const std::size_t> chosen, Unsigned low, Unsigned high)
{
    bool previous_rise = false;
    for (std::size_t i = 1; i < chosen.size(); ++i)
    {
        const auto a = values[chosen[i - 1]], b = values[chosen[i]];
        const bool rise = b > a;
        const auto delta = rise ? static_cast<Unsigned>(b) - static_cast<Unsigned>(a)
                                : static_cast<Unsigned>(a) - static_cast<Unsigned>(b);
        if (delta < low || delta > high || (i > 1 && rise == previous_rise))
        {
            return false;
        }
        previous_rise = rise;
    }
    return true;
}

Integer brute(std::span<const Integer> values, std::span<const Integer> weights, Unsigned low, Unsigned high)
{
    Integer best = 0;
    std::vector<std::size_t> chosen;
    for (Unsigned mask = 0; mask < (Unsigned{1} << values.size()); ++mask)
    {
        Integer total = 0;
        chosen.clear();
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            if (mask & (Unsigned{1} << i))
            {
                chosen.push_back(i);
                total += weights[i];
            }
        }
        if (valid(values, chosen, low, high))
        {
            best = std::max(best, total);
        }
    }
    return best;
}
}

int main()
{
    const auto none = sirkit::bounded_zigzag_subsequence({}, {}, 1, 1);
    assert(none.indices.empty() && none.weight == 0);
    const std::array<Integer, 3> heights{1, 3, 2};
    const std::array<Integer, 3> weights{5, 4, 6};
    const auto all = sirkit::bounded_zigzag_subsequence(heights, weights, 1, 3);
    assert(all.weight == 15 && all.indices == (std::vector<std::size_t>{0, 1, 2}));
    const std::array<Integer, 3> negative{-1, -2, -3};
    assert(sirkit::bounded_zigzag_subsequence(heights, negative, 1, 4).indices.empty());

    constexpr auto first = std::numeric_limits<Integer>::min();
    constexpr auto last = std::numeric_limits<Integer>::max();
    constexpr auto wide = std::numeric_limits<Unsigned>::max();
    const std::array<Integer, 3> extremes{first, last, first};
    assert(sirkit::bounded_zigzag_subsequence(extremes, weights, wide, wide).weight == 15);
    assert(sirkit::bounded_zigzag_subsequence(extremes, weights, 1, wide - 1).weight == 6);
    const std::array<Integer, 4> nearby{first + 1, first, last - 1, last};
    const std::array<Integer, 4> positive{7, 3, 5, 11};
    assert(sirkit::bounded_zigzag_subsequence(nearby, positive, 1, 2).weight == 16);
    assert(sirkit::bounded_zigzag_subsequence(nearby, positive, wide - 2, wide).weight == 18);

    std::mt19937 gen(44219);
    for (int trial = 0; trial < 1000; ++trial)
    {
        const auto n = gen() % 12;
        std::vector<Integer> values(n), scores(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            values[i] = static_cast<Integer>(gen() % 23) - 11;
            scores[i] = static_cast<Integer>(gen() % 13) - 5;
        }
        Unsigned low = (gen() % 9) + 1;
        Unsigned high = low + gen() % 11;
        const auto best = sirkit::bounded_zigzag_subsequence(values, scores, low, high);
        assert(best.weight == brute(values, scores, low, high));
        Integer verified = 0;
        for (std::size_t i = 0; i < best.indices.size(); ++i)
        {
            const auto index = best.indices[i];
            assert(index < n && (i == 0 || best.indices[i - 1] < index));
            verified += scores[index];
        }
        assert(verified == best.weight && valid(values, best.indices, low, high));
    }

    expect_exception<std::invalid_argument>([&]()
    {
        static_cast<void>(sirkit::bounded_zigzag_subsequence(heights, {}, 1, 1));
    });
    expect_exception<std::invalid_argument>([&]()
    {
        static_cast<void>(sirkit::bounded_zigzag_subsequence(heights, weights, 0, 1));
    });
    expect_exception<std::invalid_argument>([&]()
    {
        static_cast<void>(sirkit::bounded_zigzag_subsequence(heights, weights, 2, 1));
    });
    expect_exception<std::overflow_error>([&]()
    {
        const std::array<Integer, 3> excessive{last, 0, 1};
        static_cast<void>(sirkit::bounded_zigzag_subsequence(heights, excessive, 1, 2));
    });
}
