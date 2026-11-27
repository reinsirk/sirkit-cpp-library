// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Math/Tetration_Mod.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tetration_mod.md
//
// API一覧
// tetration_mod(base,height,mod): 高さheightの右結合のべき乗塔をmodで割った余り。
// 高さ0は1、0^0は1。1≤mod≤2^32−1。baseとmodが互いに素でなくてもよい。
// O(√mod + log²(mod+1))時間・O(log(mod+1))領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstdint>
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TETRATION_MOD_DEFINED
#define SIRKIT_TETRATION_MOD_DEFINED

inline std::uint32_t tetration_mod(std::uint64_t base, std::uint64_t height, std::uint32_t mod)
{
    if (mod == 0)
    {
        throw std::invalid_argument("tetration_mod: zero modulus");
    }
    if (mod == 1)
    {
        return 0;
    }
    if (height == 0 || base == 1)
    {
        return 1;
    }
    if (base == 0)
    {
        return height % 2 == 0 ? 1 : 0;
    }

    const auto reaches = [base](auto &&self, std::uint64_t h, std::uint32_t threshold) -> bool
    {
        if (threshold <= 1)
        {
            return true;
        }
        if (h == 0)
        {
            return false;
        }
        std::uint32_t needed = 0;
        for (; threshold > 1; ++needed)
        {
            threshold = static_cast<std::uint32_t>(1 + (threshold - 1) / base);
        }
        return self(self, h - 1, needed);
    };

    const auto solve = [&](auto &&self, std::uint64_t h, std::uint32_t modulus) -> std::uint32_t
    {
        if (modulus == 1)
        {
            return 0;
        }
        if (h == 1)
        {
            return static_cast<std::uint32_t>(base % modulus);
        }

        auto phi = modulus, remaining = modulus;
        for (std::uint32_t p = 2; p <= remaining / p; p += p == 2 ? 1 : 2)
        {
            if (remaining % p != 0)
            {
                continue;
            }
            phi -= phi / p;
            do
            {
                remaining /= p;
            } while (remaining % p == 0);
        }
        if (remaining != 1)
        {
            phi -= phi / remaining;
        }

        // 指数が phi 以上だった場合は、余りを取ってから phi を足す。
        std::uint64_t exponent = self(self, h - 1, phi);
        if (reaches(reaches, h - 1, phi))
        {
            exponent += phi;
        }
        auto power = base % modulus;
        std::uint64_t answer = 1;
        for (; exponent != 0; exponent >>= 1)
        {
            if (exponent & 1)
            {
                answer = answer * power % modulus;
            }
            power = power * power % modulus;
        }
        return static_cast<std::uint32_t>(answer);
    };
    return solve(solve, height, mod);
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
