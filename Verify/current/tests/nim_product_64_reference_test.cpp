// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_NIM_PRODUCT_DEFINED
#include "../../../Math/Nim_Product.cpp"
#endif
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>

namespace
{
constexpr std::size_t kSmall = 64;

std::uint64_t times_two(std::uint64_t value)
{
    // 4元体の各基底係数に2を掛ける。2ビットずつ独立に作用する。
    constexpr std::array<std::uint64_t, 4> kProduct{0, 2, 3, 1};
    std::uint64_t result = 0;
    for (int bit = 0; bit < 64; bit += 2)
    {
        result |= kProduct[(value >> bit) & 3] << bit;
    }
    return result;
}
} // namespace

int main()
{
    using sirkit::nim_product;
    std::array<std::array<std::size_t, kSmall>, kSmall> mex_product{};
    for (std::size_t a = 0; a < kSmall; ++a)
    {
        for (std::size_t b = 0; b < kSmall; ++b)
        {
            std::array<bool, 256> seen{};
            for (std::size_t x = 0; x < a; ++x)
            {
                for (std::size_t y = 0; y < b; ++y)
                {
                    seen[mex_product[x][y] ^ mex_product[a][y] ^ mex_product[x][b]] = true;
                }
            }
            std::size_t mex = 0;
            while (seen[mex])
            {
                ++mex;
            }
            mex_product[a][b] = mex;
            assert(nim_product(a, b) == mex);
        }
    }

    constexpr auto kLargest = std::numeric_limits<std::uint64_t>::max();
    assert(nim_product(kLargest, 0) == 0);
    assert(nim_product(kLargest, 1) == kLargest);
    assert(nim_product(kLargest, 2) == 0x5555555555555555ULL);
    for (int bit = 1; bit <= 32; bit *= 2)
    {
        const auto fermat = std::uint64_t{1} << bit;
        assert(nim_product(fermat, fermat) == (fermat ^ (fermat >> 1)));
        assert(nim_product(fermat, fermat - 1) == (fermat - 1) << bit);
    }

    std::mt19937_64 random(7022);
    for (int i = 0; i < 20000; ++i)
    {
        const auto a = random(), b = random(), c = random();
        assert(nim_product(a, b) == nim_product(b, a));
        assert(nim_product(a, b ^ c) == (nim_product(a, b) ^ nim_product(a, c)));
        assert(nim_product(nim_product(a, b), c) == nim_product(a, nim_product(b, c)));
        if (a != 0 && b != 0)
        {
            assert(nim_product(a, b) != 0);
        }
    }
    // 公式制約の最大クエリ数を、既知の線形写像で全件照合する。
    for (int i = 0; i < 1'000'000; ++i)
    {
        const auto value = random();
        assert(nim_product(value, 2) == times_two(value));
    }
}
