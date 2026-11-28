// SPDX-License-Identifier: CC0-1.0
#include "DataStructure/Binary_Range_Statistics.cpp"

#include <algorithm>
#include <cstdlib>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace
{
using sirkit::BinaryRangeStatistics;
using sirkit::BinaryRunStatistics;

void Expect(bool value)
{
    if (!value)
    {
        std::abort();
    }
}

template <class E, class F>
void ExpectThrow(F operation)
{
    try
    {
        operation();
    }
    catch (const E &)
    {
        return;
    }
    std::abort();
}

void CheckStats(const BinaryRunStatistics &value, std::string_view expected)
{
    Expect(value.length == static_cast<int>(expected.size()));
    Expect(value.ones == std::ranges::count(expected, '1'));
    for (int bit = 0; bit < 2; ++bit)
    {
        const auto c = static_cast<char>('0' + bit);
        const auto prefix = std::ranges::find_if(expected, [&](char x)
                                                 {
                                                     return x != c;
                                                 }) -
                            expected.begin();
        int suffix = 0;
        for (auto p = expected.rbegin(); p != expected.rend() && *p == c; ++p)
        {
            ++suffix;
        }
        int current = 0, longest = 0;
        for (char x : expected)
        {
            current = x == c ? current + 1 : 0;
            longest = std::max(longest, current);
        }
        Expect(value.prefix[bit] == prefix && value.suffix[bit] == suffix && value.longest[bit] == longest);
    }
}

void Random()
{
    std::mt19937 rng(63203103);
    for (int trial = 0; trial < 170; ++trial)
    {
        int n = rng() % 111;
        std::string bits(n, '0');
        for (auto &x : bits)
        {
            x = static_cast<char>('0' + rng() % 2);
        }
        BinaryRangeStatistics tree(bits);
        Expect(tree.size() == n);
        for (int i = 0; i < 270; ++i)
        {
            int left = rng() % (n + 1), right = rng() % (n + 1);
            if (left > right)
            {
                std::swap(left, right);
            }
            int kind = rng() % 4;
            if (kind < 2)
            {
                tree.assign(left, right, kind != 0);
                std::fill(bits.begin() + left, bits.begin() + right, static_cast<char>('0' + kind));
            }
            else if (kind == 2)
            {
                tree.flip(left, right);
                for (int j = left; j < right; ++j)
                {
                    bits[j] = bits[j] == '0' ? '1' : '0';
                }
            }
            CheckStats(tree.stats(left, right), std::string_view(bits).substr(left, right - left));
            CheckStats(tree.all_stats(), bits);
        }
    }
}

void Boundaries()
{
    BinaryRangeStatistics empty;
    Expect(empty.size() == 0);
    CheckStats(empty.stats(0, 0), "");
    BinaryRangeStatistics zeros(16);
    zeros.flip(0, 16);
    zeros.flip(0, 16);
    CheckStats(zeros.stats(1, 8), "0000000");
    zeros.assign(4, 12, true);
    zeros.flip(0, 16);
    CheckStats(zeros.all_stats(), "1111000000001111");
    auto other = zeros;
    other.assign(0, 16, false);
    CheckStats(other.all_stats(), "0000000000000000");
    CheckStats(zeros.all_stats(), "1111000000001111");
    ExpectThrow<std::length_error>([]
                                   {
                                       BinaryRangeStatistics invalid(-1);
                                   });
    ExpectThrow<std::invalid_argument>([]
                                       {
                                           BinaryRangeStatistics invalid("010x1");
                                       });
#ifndef SKIP_RUNTIME_CHECKS
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       zeros.flip(-1, 10);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)zeros.stats(6, 17);
                                   });
#endif
}
}
int main()
{
    Random();
    Boundaries();
}
