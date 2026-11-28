// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Large_Convolution.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

template <std::uint32_t Mod, std::uint32_t Root>
static void check(const std::vector<std::uint32_t> &a, const std::vector<std::uint32_t> &b, std::size_t length)
{
    const auto got = sirkit::convolution_mod_large<Mod, Root>(a, b, length);
    if (a.empty() || b.empty())
    {
        assert(got.empty());
        return;
    }
    std::vector<std::uint32_t> expected(a.size() + b.size() - 1);
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        for (std::size_t j = 0; j < b.size(); ++j)
        {
            const auto product = std::uint64_t{a[i] % Mod} * (b[j] % Mod);
            expected[i + j] = static_cast<std::uint32_t>((expected[i + j] + product) % Mod);
        }
    }
    assert(got == expected);
}

static void check_invalid_length(std::size_t length)
{
    const std::vector<std::uint32_t> a(30, 1);
    bool caught = false;
    try
    {
        (void)sirkit::convolution_mod_large<17, 3>(a, a, length);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught);
}

int main()
{
    constexpr auto kMax = std::numeric_limits<std::uint32_t>::max();
    check<17, 3>({}, {1}, 16);
    check<17, 3>({1, 0, 7}, {}, 16);
    check<17, 3>({1, 0, kMax}, {15, 16, 9, 0}, 16);
    check<17, 3>({1, kMax, 0}, {15}, 16);
    check_invalid_length(0);
    check_invalid_length(1);
    check_invalid_length(3);
    check_invalid_length(32);

    std::mt19937 rng(180217);
    for (std::size_t n = 16; n <= 48; n += 2)
    {
        for (std::size_t m = 17; m <= 45; m += 4)
        {
            std::vector<std::uint32_t> a(n), b(m);
            std::generate(a.begin(), a.end(), [&]
                          {
                              return rng();
                          });
            std::generate(b.begin(), b.end(), [&]
                          {
                              return rng();
                          });
            // 片方だけ1区画、両方を分割、1回で変換する場合を比較する。
            check<998244353, 3>(a, b, 128);
            check<998244353, 3>(a, b, 64);
            check<17, 3>(a, b, 16);
            check<2013265921, 31>(a, b, 4);
        }
    }
    std::vector<std::uint32_t> sparse(105), dense(130);
    sparse[52] = 92411;
    std::iota(dense.begin(), dense.end(), 19);
    check<998244353, 3>(sparse, dense, 32);
    check<998244353, 3>(dense, sparse, 32);
    check<998244353, 3>(std::vector<std::uint32_t>(64), dense, 32);
}
