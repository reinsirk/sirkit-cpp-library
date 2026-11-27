// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Discrete_Logarithm.cpp
// 参考（Baby-step Giant-step法）: https://flintlib.org/doc/algorithms/number-theory/discrete-log-tables.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/discrete_logarithm.md
//
// API一覧
// discrete_logarithm_mod(base,target,modulus): base^k≡target (mod modulus) の最小のk>=0。
// 解なしはstd::nullopt。0^0=1、法は1以上のuint32_t、baseとtargetは法で縮約する。
// baseと法が互いに素でなくても可。ハッシュ表が平均O(1)ならO(sqrt(modulus))時間。
// 最悪O(modulus)時間、追加O(sqrt(modulus))領域。
// 検査有効時、法が0ならstd::invalid_argument。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DISCRETE_LOGARITHM_MOD_DEFINED
#define SIRKIT_DISCRETE_LOGARITHM_MOD_DEFINED

inline std::optional<std::uint32_t> discrete_logarithm_mod(
    std::uint32_t base, std::uint32_t target, std::uint32_t modulus)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (modulus == 0)
    {
        throw std::invalid_argument("discrete_logarithm_mod: zero modulus");
    }
#endif
    target %= modulus;
    auto factor = std::uint32_t{1} % modulus;
    std::uint32_t prefix = 0;
    if (target == factor)
    {
        return prefix;
    }
    base %= modulus;

    const auto multiply = [](std::uint32_t a, std::uint32_t b, std::uint32_t mod)
    {
        return static_cast<std::uint32_t>((std::uint64_t{a} * b) % mod);
    };
    // baseと法の共通因子を除き、factor * base^k ≡ target の形にする。
    for (auto divisor = std::gcd(base, modulus); divisor > 1; divisor = std::gcd(base, modulus))
    {
        if (target % divisor != 0)
        {
            return std::nullopt;
        }
        modulus /= divisor;
        target /= divisor;
        factor = multiply(factor, base / divisor, modulus);
        ++prefix;
        if (target == factor)
        {
            return prefix;
        }
    }
    if (std::gcd(target, modulus) != 1)
    {
        return std::nullopt;
    }

    // 指数が小さい場合や周期が短い場合は表を確保せずに終える。
    auto probe = factor;
    constexpr std::uint32_t kProbeLimit = 8;
    for (std::uint32_t exponent = 1; exponent <= kProbeLimit; ++exponent)
    {
        probe = multiply(probe, base, modulus);
        if (probe == target)
        {
            return prefix + exponent;
        }
        if (probe == factor)
        {
            return std::nullopt;
        }
    }

    std::uint32_t low = 0, block_size = 65'536;
    while (block_size - low > 1)
    {
        const auto mid = low + (block_size - low) / 2;
        if (std::uint64_t{mid} * mid < modulus)
        {
            low = mid;
        }
        else
        {
            block_size = mid;
        }
    }

    const std::size_t capacity = std::bit_ceil(2U * block_size);
    const auto index = [shift = 64 - std::countr_zero(capacity)](std::uint32_t residue)
    {
        constexpr std::uint64_t kMultiplier = 0x9e3779b97f4a7c15ULL;
        return static_cast<std::size_t>((residue * kMultiplier) >> shift);
    };
    // 残余と指数+1を1枠に詰める。下位32ビットが0の枠は空。
    std::vector<std::uint64_t> offsets(capacity);
    const auto find_offset = [&](std::uint32_t residue)
    {
        auto slot = index(residue);
        while (const auto entry = offsets[slot])
        {
            if ((entry >> 32) == residue)
            {
                return static_cast<std::uint32_t>(entry);
            }
            slot = (slot + 1) & (capacity - 1);
        }
        return std::uint32_t{0};
    };
    auto baby = target;
    for (std::uint32_t offset = 0; offset < block_size; ++offset)
    {
        auto slot = index(baby);
        while (offsets[slot] != 0)
        {
            slot = (slot + 1) & (capacity - 1);
        }
        offsets[slot] = (std::uint64_t{baby} << 32) | (offset + 1);
        baby = multiply(baby, base, modulus);
        if (baby == target)
        {
            // 1ブロックで周期を一周できる。残りの指数は同じ値を繰り返す。
            const auto hit = find_offset(factor);
            if (hit == 0)
            {
                return std::nullopt;
            }
            return prefix + (offset + 2 - hit);
        }
    }

    auto multiplier = std::uint32_t{1};
    auto power = base;
    for (auto exponent = block_size; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1) != 0)
        {
            multiplier = multiply(multiplier, power, modulus);
        }
        power = multiply(power, power, modulus);
    }
    // 境界を昇順に調べると、最初に見つかったブロックに最小の指数がある。
    auto giant = factor;
    for (std::uint64_t boundary = block_size; boundary - block_size < modulus; boundary += block_size)
    {
        giant = multiply(giant, multiplier, modulus);
        const auto hit = find_offset(giant);
        if (hit != 0)
        {
            return prefix + static_cast<std::uint32_t>(boundary + 1 - hit);
        }
    }
    return std::nullopt;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
