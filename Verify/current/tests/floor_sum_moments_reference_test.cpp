// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Floor_Sum_Moments.cpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>

namespace
{
using boost::multiprecision::cpp_int;

std::uint64_t Reduce(cpp_int value)
{
    const cpp_int mask = (cpp_int(1) << 64) - 1;
    return (value & mask).convert_to<std::uint64_t>();
}

void Compare(std::uint32_t n, std::uint32_t m, std::int64_t a, std::int64_t b)
{
    cpp_int sum{}, weighted{}, pairs{}, squares{};
    for (std::uint32_t i = 0; i < n; ++i)
    {
        const cpp_int value = cpp_int(a) * i + b;
        const cpp_int q = value >= 0 ? cpp_int(value / m) : cpp_int((value - (m - 1)) / m);
        sum += q;
        weighted += i * q;
        pairs += q * (q - 1) / 2;
        squares += q * q;
    }
    const auto actual = sirkit::floor_sum_moments(n, m, a, b);
    assert(actual.sum == Reduce(sum));
    assert(actual.index_weighted == Reduce(weighted));
    assert(actual.pairs == Reduce(pairs));
    assert(actual.sum_of_squares() == Reduce(squares));
}
}

int main()
{
    using sirkit::floor_sum_moments;
    try
    {
        static_cast<void>(floor_sum_moments(3, 0, 1, 0));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }

    for (std::uint32_t n = 0; n <= 8; ++n)
    {
        for (std::uint32_t m = 1; m <= 8; ++m)
        {
            for (std::int64_t a = -8; a <= 8; ++a)
            {
                for (std::int64_t b = -8; b <= 8; ++b)
                {
                    Compare(n, m, a, b);
                }
            }
        }
    }
    constexpr auto kMax = std::numeric_limits<std::uint32_t>::max();
    constexpr auto kMin64 = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax64 = std::numeric_limits<std::int64_t>::max();
    Compare(135, kMax, kMin64, kMax64);
    Compare(134, kMax, kMax64, kMin64);
    Compare(133, 1, kMin64, kMin64);
    Compare(132, 1, kMax64, kMax64);

    const auto maximum = floor_sum_moments(kMax, kMax, kMax - 1, kMax - 1);
    // floor(((m-1)i+(m-1))/m)=i for 0<=i<m.
    const cpp_int n = kMax;
    const cpp_int linear = n * (n - 1) / 2;
    const cpp_int quadratic = n * (n - 1) * (2 * n - 1) / 6;
    assert(maximum.sum == Reduce(linear));
    assert(maximum.index_weighted == Reduce(quadratic));
    assert(maximum.sum_of_squares() == Reduce(quadratic));
    assert(maximum.pairs == Reduce((quadratic - linear) / 2));
}
