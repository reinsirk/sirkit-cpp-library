// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_MONGE_OPTIMIZATION_TEST_SOURCE
#define SIRKIT_MONGE_OPTIMIZATION_TEST_SOURCE "../../../Algorithm/Monge_Optimization.cpp"
#endif
#include SIRKIT_MONGE_OPTIMIZATION_TEST_SOURCE
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
std::vector<int> NaiveMinPlus(const std::vector<int> &a, const std::vector<int> &b)
{
    if (a.empty() || b.empty())
    {
        return {};
    }
    std::vector<int> result(a.size() + b.size() - 1, std::numeric_limits<int>::max());
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        for (std::size_t j = 0; j < b.size(); ++j)
        {
            result[i + j] = std::min(result[i + j], a[i] + b[j]);
        }
    }
    return result;
}

std::vector<int> RandomSequence(int size, std::mt19937 &random, bool convex)
{
    std::vector<int> values(size);
    for (int &x : values)
    {
        x = static_cast<int>(random() % 31) - 15;
    }
    if (convex)
    {
        std::sort(values.begin(), values.end());
        for (int i = 1; i < size; ++i)
        {
            values[i] += values[i - 1];
        }
    }
    return values;
}
} // namespace

int main()
{
    std::mt19937 random(67279);
    for (int height = 0; height <= 16; ++height)
    {
        for (int width = 0; width <= 28; ++width)
        {
            for (int repetition = 0; repetition < 12; ++repetition)
            {
                const auto bias = RandomSequence(width, random, false);
                const int slope = repetition % 3;
                const int offset = static_cast<int>(random() % 19) - 9;
                const auto value = [&](int row, int col)
                {
                    const int delta = row - col + offset;
                    return slope * delta * delta + bias[col];
                };
                const auto better = [&](int row, int current, int candidate)
                {
                    return value(row, candidate) < value(row, current);
                };
                std::vector<int> expected(height, width == 0 ? -1 : 0);
                for (int row = 0; row < height; ++row)
                {
                    for (int col = 1; col < width; ++col)
                    {
                        if (better(row, expected[row], col))
                        {
                            expected[row] = col;
                        }
                    }
                }
                assert(sirkit::monotone_minima(height, width, better) == expected);
                assert(sirkit::smawk(height, width, better) == expected);
            }
        }
    }

    for (int n = 0; n <= 20; ++n)
    {
        for (int m = 0; m <= 24; ++m)
        {
            for (int repetition = 0; repetition < 12; ++repetition)
            {
                const auto a = RandomSequence(n, random, true);
                const auto b = RandomSequence(m, random, true);
                const auto arbitrary = RandomSequence(m, random, false);
                assert(sirkit::min_plus_convolution_convex_convex(a, b) == NaiveMinPlus(a, b));
                assert(sirkit::min_plus_convolution_convex_arbitrary(a, arbitrary) == NaiveMinPlus(a, arbitrary));
            }
        }
    }

    const auto rejects_length = [](std::size_t n, std::size_t m)
    {
        try
        {
            sirkit::detail::CheckedMinPlusLength(n, m);
        }
        catch (const std::length_error &)
        {
            return true;
        }
        return false;
    };
    constexpr auto kLimit = static_cast<std::size_t>(std::numeric_limits<int>::max());
    assert(rejects_length(kLimit + 1, 1));
    assert(rejects_length(kLimit, 2));
    assert(sirkit::detail::CheckedMinPlusLength(kLimit, 1) == std::numeric_limits<int>::max());
#ifndef SKIP_RUNTIME_CHECKS
    const auto rejects_dimensions = [](int height, int width)
    {
        try
        {
            sirkit::detail::CheckMongeDimensions(height, width);
        }
        catch (const std::invalid_argument &)
        {
            return true;
        }
        return false;
    };
    assert(rejects_dimensions(-1, 0));
    assert(rejects_dimensions(0, -1));
#endif
}
