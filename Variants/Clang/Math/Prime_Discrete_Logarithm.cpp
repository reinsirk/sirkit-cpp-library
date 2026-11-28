// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Prime_Discrete_Logarithm.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/prime_discrete_logarithm.md
//
// API一覧
// [PrimeDiscreteLogarithm]
// PrimeDiscreteLogarithm(prime, generator, expected_queries=1): uint32_tの素数とその原始根を固定する。
// log(value): generator^x=value mod prime となる0<=x<prime-1を返す。1<=value<prime。
// Bは前計算する累乗の個数。O(B+sqrt(prime))の期待構築時間、1問O(prime/B+1)の期待時間。
// 保持領域O(B)。密な表には存在ビットと累積popcountを使う。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#if !defined(SKIP_RUNTIME_CHECKS) && !defined(SIRKIT_PRIMALITY32_DEFINED)
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline bool is_prime_u32(std::uint32_t n) noexcept;
}
#endif

#ifndef SIRKIT_PRIME_DISCRETE_LOGARITHM_DEFINED
#define SIRKIT_PRIME_DISCRETE_LOGARITHM_DEFINED
class PrimeDiscreteLogarithm
{
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    struct Multiplier
    {
        u32 value;
        u32 reciprocal;
    };

    u32 prime_;
    u32 steps_;
    Multiplier jump_;
    int hash_shift_ = 0;
    std::vector<u64> sparse_;
    std::vector<u64> present_;
    std::vector<u32> ranks_;
    std::vector<u32> exponents_;

    u32 multiply(u32 left, u32 right) const noexcept
    {
        return static_cast<u32>(static_cast<u64>(left) * right % prime_);
    }

    Multiplier multiplier(u32 value) const noexcept
    {
        return {value, static_cast<u32>((static_cast<u64>(value) << 32) / prime_)};
    }

    // 固定乗数に対するShoup乗算。商の近似は真値より高々1小さい。
    u32 multiply(u32 left, Multiplier right) const noexcept
    {
        const u64 quotient = (static_cast<u64>(left) * right.reciprocal) >> 32;
        const u64 residue = static_cast<u64>(left) * right.value - quotient * prime_;
        return static_cast<u32>(residue >= prime_ ? residue - prime_ : residue);
    }

    u32 power(u32 base, u32 exponent) const noexcept
    {
        u32 result = 1;
        for (; exponent != 0; exponent >>= 1)
        {
            if ((exponent & 1u) != 0)
            {
                result = multiply(result, base);
            }
            base = multiply(base, base);
        }
        return result;
    }

#ifndef SKIP_RUNTIME_CHECKS
    bool is_generator(u32 base) const noexcept
    {
        const u32 order = prime_ - 1;
        u32 rest = order;
        for (u32 divisor = 2; static_cast<u64>(divisor) * divisor <= rest; ++divisor)
        {
            if (rest % divisor != 0)
            {
                continue;
            }
            if (power(base, order / divisor) == 1)
            {
                return false;
            }
            do
            {
                rest /= divisor;
            } while (rest % divisor == 0);
        }
        return rest == 1 || power(base, order / rest) != 1;
    }
#endif

    std::size_t hash(u32 key) const noexcept
    {
        constexpr u64 kHashMultiplier = 0x9e3779b97f4a7c15ULL;
        return static_cast<std::size_t>((kHashMultiplier * key) >> hash_shift_);
    }

    void build_sparse(Multiplier generator, std::size_t capacity)
    {
        sparse_.resize(capacity);
        const auto mask = capacity - 1;
        hash_shift_ = 64 - static_cast<int>(std::bit_width(mask));
        u32 residue = 1;
        for (u32 exponent = 0; exponent < steps_; ++exponent)
        {
            auto position = hash(residue);
            while (sparse_[position] != 0)
            {
                position = (position + 1) & mask;
            }
            sparse_[position] = (static_cast<u64>(residue) << 32) | exponent;
            residue = multiply(residue, generator);
        }
    }

    u32 rank(u32 value, u64 word) const noexcept
    {
        const u64 mask = (u64{1} << (value & 63u)) - 1;
        return ranks_[value >> 6] + static_cast<u32>(std::popcount(word & mask));
    }

    void build_dense(Multiplier generator, std::size_t word_count)
    {
        present_.resize(word_count);
        ranks_.resize(word_count);
        exponents_.resize(steps_);
        u32 residue = 1;
        for (u32 exponent = 0; exponent < steps_; ++exponent)
        {
            present_[residue >> 6] |= u64{1} << (residue & 63u);
            residue = multiply(residue, generator);
        }
        u32 offset = 0;
        for (std::size_t index = 0; index < word_count; ++index)
        {
            ranks_[index] = offset;
            offset += static_cast<u32>(std::popcount(present_[index]));
        }
        residue = 1;
        for (u32 exponent = 0; exponent < steps_; ++exponent)
        {
            exponents_[rank(residue, present_[residue >> 6])] = exponent;
            residue = multiply(residue, generator);
        }
    }

    u32 lookup(u32 value) const noexcept
    {
        if (!present_.empty())
        {
            const auto word = present_[value >> 6];
            if (((word >> (value & 63u)) & 1u) != 0)
            {
                return exponents_[rank(value, word)];
            }
            return steps_;
        }
        const auto mask = sparse_.size() - 1;
        auto position = hash(value);
        for (auto slot = sparse_[position]; slot != 0; slot = sparse_[position])
        {
            if (slot >> 32 == value)
            {
                return static_cast<u32>(slot);
            }
            position = (position + 1) & mask;
        }
        return steps_;
    }

public:
    PrimeDiscreteLogarithm(u32 prime, u32 generator, u64 expected_queries = 1) : prime_(prime)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (generator == 0 || generator >= prime || !detail::is_prime_u32(prime))
        {
            throw std::invalid_argument("PrimeDiscreteLogarithm: prime or generator");
        }
        if (!is_generator(generator))
        {
            throw std::invalid_argument("PrimeDiscreteLogarithm: primitive root required");
        }
#endif
        const auto order = static_cast<u64>(prime - 1);
        const auto effort = std::clamp(expected_queries, u64{1}, order);
        steps_ = static_cast<u32>(std::min(order, static_cast<u64>(std::sqrt(static_cast<double>(order * effort)))));
        jump_ = multiplier(power(generator, static_cast<u32>(order) - steps_));
        const auto generator_multiplier = multiplier(generator);
        const auto capacity = std::bit_ceil(2 * static_cast<u64>(steps_));
        const auto words = (static_cast<u64>(prime) + 63) / 64;
        // 保持バイト数が小さくなる方を選ぶ。疎な表の負荷率は最大1/2。
        if (words * (sizeof(u64) + sizeof(u32)) + steps_ * sizeof(u32) < capacity * sizeof(u64))
        {
            build_dense(generator_multiplier, static_cast<std::size_t>(words));
        }
        else
        {
            build_sparse(generator_multiplier, static_cast<std::size_t>(capacity));
        }
    }

    [[nodiscard]] u32 log(u32 value) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (value == 0 || value >= prime_)
        {
            throw std::out_of_range("PrimeDiscreteLogarithm: nonzero residue required");
        }
#endif
        for (u32 offset = 0;; offset += steps_)
        {
            const auto exponent = lookup(value);
            if (exponent != steps_)
            {
                return offset + exponent;
            }
            value = multiply(value, jump_);
        }
    }
};
#endif
#if !defined(SKIP_RUNTIME_CHECKS) && !defined(SIRKIT_PRIMALITY32_DEFINED)
#define SIRKIT_PRIMALITY32_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
// uint32_t全域の素数判定。Miller-Rabinの底2,7,61を使う。
inline bool is_prime_u32(std::uint32_t n) noexcept
{
    // 競技プログラミングでよく使う素数は、累乗計算を省く。
    switch (n)
    {
    case 167772161u:
    case 469762049u:
    case 754974721u:
    case 998244353u:
    case 1000000007u:
    case 1000000009u:
    case 1004535809u:
    case 1224736769u:
    case 2013265921u:
    case 61u: // Miller–Rabinの底のため先に処理する。
        return true;
    default:
        break;
    }
    if (n < 2)
        return false;
    if (n % 2u == 0) return n == 2u;
    if (n % 3u == 0) return n == 3u;
    if (n % 5u == 0) return n == 5u;
    if (n % 7u == 0) return n == 7u;
    if (n % 11u == 0) return n == 11u;
    std::uint32_t odd = n - 1;
    unsigned shifts = 0;
    while ((odd & 1u) == 0)
    {
        odd >>= 1;
        ++shifts;
    }
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
    // R=2^64。両引数がn以下なら積はR未満で、還元結果はn以下。
    // nも0の表現として保持でき、1や-1の判定に影響しない。
    using u64 = std::uint64_t;
    using u128 = __uint128_t; // GCC/Clangの128ビット整数拡張。
    u64 inverse = n;
    for (int i = 0; i < 5; ++i) inverse *= 2u - u64(n) * inverse;
    const u64 negative_inverse = u64{0} - inverse;
    const auto multiply = [n, negative_inverse](u64 a, u64 b) {
        const u128 product = static_cast<u128>(a) * b;
        const u128 correction = static_cast<u128>(static_cast<u64>(product) * negative_inverse) * n;
        return static_cast<u64>((product + correction) >> 64);
    };
    const u64 one = (u64{0} - n) % n;
    const u64 r_squared = one * one % n;
    for (std::uint32_t base : {2u, 7u, 61u}) {
        u64 value = one, power = multiply(base % n, r_squared);
        for (std::uint32_t exponent = odd; exponent; exponent >>= 1) {
            if (exponent & 1u) value = multiply(value, power);
            power = multiply(power, power);
        }
        if (value == one || value == n - one) continue;
        unsigned step = 1;
        for (; step < shifts; ++step) {
            value = multiply(value, value);
            if (value == n - one) break;
        }
        if (step == shifts) return false;
    }
#else
    // R=2^32のMontgomery乗算。33ビットの還元結果もuint64_tで扱う。
    std::uint32_t inverse = n;
    for (int i = 0; i < 5; ++i)
        inverse *= 2u - n * inverse;
    const std::uint32_t negative_inverse = 0u - inverse;
    const auto multiply = [n, negative_inverse](std::uint32_t a, std::uint32_t b)
    {
        const std::uint64_t product = static_cast<std::uint64_t>(a) * b;
        const std::uint32_t factor = static_cast<std::uint32_t>(product) * negative_inverse;
        const std::uint64_t correction = static_cast<std::uint64_t>(factor) * n;
        std::uint64_t result = (product >> 32) + (correction >> 32) +
                               (static_cast<std::uint32_t>(product) != 0);
        if (result >= n)
            result -= n;
        return static_cast<std::uint32_t>(result);
    };
    const auto one = static_cast<std::uint32_t>((std::uint64_t{1} << 32) % n);
    const auto r_squared = static_cast<std::uint32_t>(static_cast<std::uint64_t>(one) * one % n);
    for (std::uint32_t base : {2u, 7u, 61u})
    {
        std::uint32_t value = one, power = multiply(base % n, r_squared);
        for (std::uint32_t exponent = odd; ;)
        {
            if (exponent & 1u)
                value = multiply(value, power);
            exponent >>= 1;
            if (exponent == 0) break;
            power = multiply(power, power);
        }
        if (value == one || value == n - one)
            continue;
        unsigned step = 1;
        for (; step < shifts; ++step)
        {
            value = multiply(value, value);
            if (value == n - one)
                break;
        }
        if (step == shifts)
            return false;
    }
#endif
    return true;
}
} // namespace reinsirk::detail
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
