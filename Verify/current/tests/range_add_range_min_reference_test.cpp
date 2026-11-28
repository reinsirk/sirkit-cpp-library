// SPDX-License-Identifier: CC0-1.0
#include "DataStructure/Range_Add_Range_Min.cpp"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
void Expect(bool condition)
{
    if (!condition)
    {
        std::abort();
    }
}

template <class Error, class Op>
void ExpectThrow(Op operation)
{
    try
    {
        operation();
    }
    catch (const Error &)
    {
        return;
    }
    std::abort();
}

void Random()
{
    std::mt19937 rng(21099131);
    for (int trial = 0; trial < 160; ++trial)
    {
        const std::size_t n = 1 + rng() % 70;
        std::vector<long long> model(n);
        for (auto &x : model)
        {
            x = static_cast<int>(rng() % 4000) - 2000;
        }
        sirkit::RangeAddRangeMin<long long> tree(model);
        for (int i = 0; i < 380; ++i)
        {
            auto a = rng() % (n + 1), b = rng() % (n + 1);
            if (a > b)
            {
                std::swap(a, b);
            }
            if (i % 3 != 0)
            {
                const int value = static_cast<int>(rng() % 701) - 350;
                tree.add(a, b, value);
                for (std::size_t j = a; j < b; ++j)
                {
                    model[j] += value;
                }
            }
            auto actual = tree.minimum(a, b);
            if (a == b)
            {
                Expect(!actual);
            }
            else
            {
                Expect(actual == *std::min_element(model.begin() + a, model.begin() + b));
            }
            Expect(tree.all_minimum() == *std::min_element(model.begin(), model.end()));
        }
    }
}

void Boundaries()
{
    sirkit::RangeAddRangeMin<long long> empty;
    Expect(empty.size() == 0 && !empty.all_minimum() && !empty.minimum(0, 0));
    empty.add(0, 0, -1);
    const sirkit::RangeAddRangeMin<long long> empty_span(std::span<const long long>{});
    Expect(!empty_span.all_minimum());
    const auto low = std::numeric_limits<long long>::min(), high = std::numeric_limits<long long>::max();
    const std::vector<long long> init{low + 5, high - 5};
    sirkit::RangeAddRangeMin<long long> extreme(init);
    extreme.add(0, 1, -5);
    extreme.add(1, 2, 5);
    Expect(extreme.all_minimum() == low && extreme.minimum(1, 2) == high);
    const auto copied = extreme;
    extreme = sirkit::RangeAddRangeMin<long long>(23);
    extreme.add(0, 23, 7);
    extreme.add(8, 12, -5);
    Expect(extreme.all_minimum() == 2 && extreme.minimum(3, 8) == 7 && copied.all_minimum() == low);
    ExpectThrow<std::length_error>([]
                                   {
                                       sirkit::RangeAddRangeMin<long long> oversize(std::numeric_limits<std::size_t>::max());
                                   });
#ifndef SKIP_RUNTIME_CHECKS
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       extreme.add(30, 28, -5);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)extreme.minimum(20, 24);
                                   });
#endif
}
}

int main()
{
    Random();
    Boundaries();
}
