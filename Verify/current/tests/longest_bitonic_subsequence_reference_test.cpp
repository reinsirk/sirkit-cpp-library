#include "../../../Algorithm/Longest_Bitonic_Subsequence.cpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
template <bool Strict, typename T>
bool rises(const T &left, const T &right)
{
    if constexpr (Strict)
    {
        return left < right;
    }
    return !(right < left);
}

template <bool Strict>
std::size_t enumerate(const std::vector<int> &values)
{
    std::size_t longest = 0;
    for (unsigned mask = 0; mask < (1U << values.size()); ++mask)
    {
        const auto length = static_cast<std::size_t>(std::popcount(mask));
        if (length <= longest)
        {
            continue;
        }
        bool falling = false, valid = true;
        std::size_t previous = values.size();
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            if ((mask & (1U << i)) == 0)
            {
                continue;
            }
            if (previous != values.size())
            {
                if (values[previous] < values[i])
                {
                    valid &= !falling;
                }
                else if (values[i] < values[previous])
                {
                    falling = true;
                }
                else
                {
                    valid &= !Strict;
                }
            }
            previous = i;
        }
        if (valid)
        {
            longest = length;
        }
    }
    return longest;
}

template <bool Strict = true, typename T>
void verify(const std::vector<T> &values, std::size_t length)
{
    const auto result = sirkit::longest_bitonic_subsequence<Strict>(values);
    assert(result.indices.size() == length);
    if (length == 0)
    {
        assert(result.peak == 0);
        return;
    }
    assert(result.peak < length);
    for (std::size_t i = 0; i < result.indices.size(); ++i)
    {
        assert(result.indices[i] < values.size());
        if (i == 0)
        {
            continue;
        }
        assert(result.indices[i - 1] < result.indices[i]);
        const auto &left = values[result.indices[i - 1]];
        const auto &right = values[result.indices[i]];
        assert(i <= result.peak ? rises<Strict>(left, right) : rises<Strict>(right, left));
    }
}
}

int main()
{
    verify(std::vector<int>{}, 0);
    verify(std::vector{1, 2, 3, 3, 2, 1}, 5);
    verify<false>(std::vector{1, 2, 3, 3, 2, 1}, 6);
    verify(std::vector{1, 2, 3, 4}, 4);
    verify(std::vector{4, 3, 2, 1}, 4);
    verify(std::vector{3, 3, 3, 3}, 1);
    verify<false>(std::vector{3, 3, 3, 3}, 4);
    verify(std::vector<std::string>{"bb", "aaa", "cc", "ccc", "dd", "baa"}, 5);
    verify(std::vector<bool>{false, true, true, false}, 3);
    verify<false>(std::vector<bool>{false, true, true, false}, 4);
    verify(std::vector{0.25, 0.5, 0.125}, 3);

    std::mt19937 random(60251);
    for (int trial = 0; trial < 1600; ++trial)
    {
        std::vector<int> values(random() % 14);
        for (auto &value : values)
        {
            value = static_cast<int>(random() % 11) - 5;
        }
        verify(values, enumerate<true>(values));
        verify<false>(values, enumerate<false>(values));
    }

#ifndef SKIP_RUNTIME_CHECKS
    bool caught = false;
    try
    {
        (void)sirkit::longest_bitonic_subsequence(std::vector{std::numeric_limits<double>::quiet_NaN()});
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught);
#endif
}
