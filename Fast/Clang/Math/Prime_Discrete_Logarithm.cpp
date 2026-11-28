// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
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

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
