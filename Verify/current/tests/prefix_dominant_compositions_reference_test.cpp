// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Prefix_Dominant_Compositions.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace
{
template <std::uint32_t Mod>
void CheckSmall(std::size_t length, std::size_t p, std::size_t q)
{
    const auto result = sirkit::prefix_dominant_composition_counts<Mod>(length, p, q);
    assert(result.size() == length + 1 && result[0] == 0);

    // 二次元の中間和ごとに、次の非負の列を直接加える。
    std::vector<std::vector<std::uint64_t>> current(p + 1, std::vector<std::uint64_t>(q + 1));
    current[0][0] = 1;
    for (std::size_t used = 1; used <= length; ++used)
    {
        std::vector<std::vector<std::uint64_t>> next(p + 1, std::vector<std::uint64_t>(q + 1));
        for (std::size_t x = 0; x <= p; ++x)
        {
            for (std::size_t y = 0; y <= q; ++y)
            {
                for (auto u = x; u <= p; ++u)
                {
                    for (auto v = y; v <= q; ++v)
                    {
                        if (u * q >= v * p)
                        {
                            next[u][v] += current[x][y];
                        }
                    }
                }
            }
        }
        current.swap(next);
        assert(result[used] == current[p][q] % Mod);
    }
}

template <class Error, class Call>
void ExpectError(Call call)
{
    bool rejected = false;
    try
    {
        call();
    }
    catch (const Error &)
    {
        rejected = true;
    }
    assert(rejected);
}
} // namespace

int main()
{
    for (std::size_t p = 1; p <= 6; ++p)
    {
        for (std::size_t q = 1; q <= 7; ++q)
        {
            if (std::gcd(p, q) == 1)
            {
                CheckSmall<998244353>(8, p, q);
                CheckSmall<7>(6, p, q);
                CheckSmall<2>(1, p, q);
            }
        }
    }
    CheckSmall<998244353>(0, 1, 1);
    constexpr auto huge = std::numeric_limits<std::uint64_t>::max();
    const std::vector<std::uint32_t> expected{0, 1, 301, 32801, 1934051, 73801301, 2019008201, 3657874082, 2804074811};
    assert(sirkit::prefix_dominant_composition_counts<4294967291>(8, huge, huge - 1) == expected);
    ExpectError<std::invalid_argument>([]
                                       {
                                           (void)sirkit::prefix_dominant_composition_counts(1, 0, 1);
                                       });
    ExpectError<std::invalid_argument>([]
                                       {
                                           (void)sirkit::prefix_dominant_composition_counts(1, 1, 0);
                                       });
    ExpectError<std::invalid_argument>([]
                                       {
                                           (void)sirkit::prefix_dominant_composition_counts(1, 6, 9);
                                       });
    ExpectError<std::length_error>([]
                                   {
                                       (void)sirkit::prefix_dominant_composition_counts<7>(7, 1, 1);
                                   });
}
