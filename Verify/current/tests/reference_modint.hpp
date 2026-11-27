// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_VERIFY_REFERENCE_MODINT_HPP
#define SIRKIT_VERIFY_REFERENCE_MODINT_HPP

#include <concepts>
#include <cstdint>

namespace sirkit_verify
{
// 独立参照で使う素数法。64ビット乗算だけで32ビット処理系でも動く。
template <std::uint32_t Prime = 0>
class ReferenceModInt
{
public:
    constexpr ReferenceModInt() = default;

    template <std::integral I>
    ReferenceModInt(I value)
    {
        if constexpr (std::signed_integral<I>)
        {
            const auto remainder = static_cast<std::int64_t>(value) % mod_;
            value_ = static_cast<std::uint32_t>(remainder < 0 ? remainder + mod_ : remainder);
        }
        else
        {
            value_ = static_cast<std::uint32_t>(static_cast<std::uint64_t>(value) % mod_);
        }
    }

    static void set_mod(std::uint32_t prime)
        requires(Prime == 0)
    {
        mod_ = prime;
    }

    ReferenceModInt &operator+=(ReferenceModInt other)
    {
        const std::uint64_t sum = std::uint64_t{value_} + other.value_;
        value_ = static_cast<std::uint32_t>(sum >= mod_ ? sum - mod_ : sum);
        return *this;
    }

    ReferenceModInt &operator-=(ReferenceModInt other)
    {
        const std::uint64_t adjusted = value_ < other.value_ ? std::uint64_t{value_} + mod_ : value_;
        value_ = static_cast<std::uint32_t>(adjusted - other.value_);
        return *this;
    }

    ReferenceModInt &operator*=(ReferenceModInt other)
    {
        value_ = static_cast<std::uint32_t>(std::uint64_t{value_} * other.value_ % mod_);
        return *this;
    }

    ReferenceModInt pow(std::uint32_t exponent) const
    {
        ReferenceModInt result{1}, base = *this;
        for (; exponent != 0; exponent >>= 1)
        {
            if (exponent & 1)
            {
                result *= base;
            }
            base *= base;
        }
        return result;
    }

    friend bool operator==(ReferenceModInt, ReferenceModInt) = default;
    friend ReferenceModInt operator+(ReferenceModInt a, ReferenceModInt b)
    {
        return a += b;
    }
    friend ReferenceModInt operator-(ReferenceModInt a, ReferenceModInt b)
    {
        return a -= b;
    }
    friend ReferenceModInt operator*(ReferenceModInt a, ReferenceModInt b)
    {
        return a *= b;
    }
    friend ReferenceModInt operator/(ReferenceModInt a, ReferenceModInt b)
    {
        return a *= b.pow(mod_ - 2);
    }

private:
    inline static std::uint32_t mod_ = Prime != 0 ? Prime : 998244353;
    std::uint32_t value_ = 0;
};
} // namespace sirkit_verify

#endif
