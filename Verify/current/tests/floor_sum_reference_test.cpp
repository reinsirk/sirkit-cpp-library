// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_FLOOR_SUM_TEST_SOURCE
#define SIRKIT_FLOOR_SUM_TEST_SOURCE "../../../Math/Floor_Sum.cpp"
#endif
#include SIRKIT_FLOOR_SUM_TEST_SOURCE
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>

int main()
{
    for (std::int64_t n = 0; n <= 15; ++n)
    {
        for (std::int64_t m = 1; m <= 15; ++m)
        {
            for (std::int64_t a = -20; a <= 20; ++a)
            {
                for (std::int64_t b = -20; b <= 20; ++b)
                {
                    std::int64_t expected = 0, minimum = m;
                    for (std::int64_t i = 0; i < n; ++i)
                    {
                        const auto term = a * i + b;
                        expected += term / m - (term % m < 0);
                        minimum = std::min(minimum, (term % m + m) % m);
                    }
                    assert(sirkit::floor_sum(n, m, a, b) == expected);
                    if (n != 0)
                    {
                        assert(sirkit::min_of_mod_of_linear(n, m, a, b) == minimum);
                    }
                }
            }
        }
    }
    constexpr auto kLimit = std::int64_t{1} << 32;
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    constexpr auto kN = kLimit - 1;
    assert(sirkit::floor_sum(kN, kN, kN - 1, kN - 1) == kN * ((kN - 1) / 2));
    assert(sirkit::floor_sum(3, 1, kMin, kMin) == 0);
    assert(sirkit::floor_sum(2, 1, kMax, kMax) == kMax - 2);
    assert(sirkit::floor_sum(1, 3, kMin, kMin) == kMin / 3 - 1);
    assert(sirkit::min_of_mod_of_linear(kN, kN, kN - 1, kN - 1) == 0);
    assert(sirkit::min_of_mod_of_linear(kN, kN - 1, 4, 3) == 1);
    assert(sirkit::min_of_mod_of_linear(1, 3, kMin, kMin) == 1);
#ifndef SKIP_RUNTIME_CHECKS
    const auto rejects = [](std::int64_t n, std::int64_t m)
    {
        try
        {
            sirkit::floor_sum(n, m, 0, 0);
        }
        catch (const std::invalid_argument &)
        {
            return true;
        }
        return false;
    };
    assert(rejects(-1, 1));
    assert(rejects(kLimit, 1));
    assert(rejects(3, 0));
    assert(rejects(3, kLimit));
    const auto rejects_minimum = [](std::int64_t n, std::int64_t m)
    {
        try
        {
            sirkit::min_of_mod_of_linear(n, m, 0, 0);
        }
        catch (const std::invalid_argument &)
        {
            return true;
        }
        return false;
    };
    assert(rejects_minimum(0, 1));
    assert(rejects_minimum(kLimit, 1));
    assert(rejects_minimum(3, 0));
    assert(rejects_minimum(3, kLimit));
#endif
}
