#include <compare>
#include <string>
#include <string_view>

#ifndef SIRKIT_BIG_INTEGER_DEFINED
#define SIRKIT_BIG_INTEGER_DEFINED
template <unsigned Radix = 10>
class BigInteger
{
    static_assert(Radix == 10 || Radix == 16);
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;
    using i64 = std::int64_t;

    static constexpr u32 kBlockSize = 4;
    static constexpr u32 kBase = Radix * Radix * Radix * Radix;
    static constexpr u32 kFirstPrime = 998244353;
    static constexpr u32 kSecondPrime = 469762049;
    static constexpr u32 kFirstInverseModuloSecond = 208783132;
    static constexpr std::string_view kDigits = "0123456789ABCDEF";
    static constexpr std::size_t kDivisionLeafSize = 64;

    std::vector<u32> limbs_;
    bool negative_ = false;

    void trim() noexcept
    {
        while (!limbs_.empty() && limbs_.back() == 0)
        {
            limbs_.pop_back();
        }
        if (limbs_.empty())
        {
            negative_ = false;
        }
    }

    static unsigned parse_digit(char character) noexcept
    {
        if ('0' <= character && character <= '9')
        {
            return static_cast<unsigned>(character - '0');
        }
        if constexpr (Radix == 16)
        {
            if ('A' <= character && character <= 'F')
            {
                return 10u + static_cast<unsigned>(character - 'A');
            }
            if ('a' <= character && character <= 'f')
            {
                return 10u + static_cast<unsigned>(character - 'a');
            }
        }
        return Radix;
    }

    static int magnitude_order(const BigInteger &left, const BigInteger &right) noexcept
    {
        if (left.limbs_.size() != right.limbs_.size())
        {
            return left.limbs_.size() < right.limbs_.size() ? -1 : 1;
        }
        for (std::size_t i = left.limbs_.size(); i != 0; --i)
        {
            const auto a = left.limbs_[i - 1];
            const auto b = right.limbs_[i - 1];
            if (a != b)
            {
                return a < b ? -1 : 1;
            }
        }
        return 0;
    }

    static BigInteger add_or_subtract(const BigInteger &left, const BigInteger &right, bool subtract)
    {
        const bool right_negative = right.negative_ != subtract;
        if (left.limbs_.empty())
        {
            auto out = right;
            out.negative_ = !out.limbs_.empty() && right_negative;
            return out;
        }
        if (right.limbs_.empty())
        {
            return left;
        }
        BigInteger out;
        if (left.negative_ == right_negative)
        {
            const auto count = std::max(left.limbs_.size(), right.limbs_.size());
            out.limbs_.resize(count);
            out.negative_ = left.negative_;
            u32 carry = 0;
            for (std::size_t i = 0; i < count; ++i)
            {
                const u32 a = i < left.limbs_.size() ? left.limbs_[i] : 0;
                const u32 b = i < right.limbs_.size() ? right.limbs_[i] : 0;
                const u32 sum = a + b + carry;
                carry = sum >= kBase;
                out.limbs_[i] = sum - carry * kBase;
            }
            if (carry != 0)
            {
                out.limbs_.push_back(carry);
            }
            return out;
        }

        const auto order = magnitude_order(left, right);
        if (order == 0)
        {
            return {};
        }
        const auto &bigger = order > 0 ? left.limbs_ : right.limbs_;
        const auto &smaller = order > 0 ? right.limbs_ : left.limbs_;
        out.limbs_.resize(bigger.size());
        out.negative_ = order > 0 ? left.negative_ : right_negative;
        std::int32_t carry = 0;
        for (std::size_t i = 0; i < bigger.size(); ++i)
        {
            const auto sub = i < smaller.size() ? smaller[i] : 0;
            const auto value = static_cast<std::int32_t>(bigger[i]) - static_cast<std::int32_t>(sub) - carry;
            carry = value < 0;
            out.limbs_[i] = static_cast<u32>(value + carry * static_cast<std::int32_t>(kBase));
        }
        out.trim();
        return out;
    }

    static u64 restore_coefficient(u32 first, u32 second) noexcept
    {
        const u32 reduced = first % kSecondPrime;
        const u32 delta = second >= reduced ? second - reduced : second + kSecondPrime - reduced;
        const u64 lifted = static_cast<u64>(delta) * kFirstInverseModuloSecond % kSecondPrime;
        return first + static_cast<u64>(kFirstPrime) * lifted;
    }

    template <class Coefficient>
    static std::vector<u32> carry_digits(std::size_t count, const Coefficient &coefficient)
    {
        std::vector<u32> result(count);
        u64 carry = 0;
        for (std::size_t i = 0; i < count; ++i)
        {
            const auto value = coefficient(i) + carry;
            result[i] = static_cast<u32>(value % kBase);
            carry = value / kBase;
        }
        while (carry != 0)
        {
            result.push_back(static_cast<u32>(carry % kBase));
            carry /= kBase;
        }
        return result;
    }

    static std::vector<u32> multiply_magnitudes(const std::vector<u32> &left, const std::vector<u32> &right)
    {
        const auto size = left.size() + right.size() - 1;
        if (std::min(left.size(), right.size()) <= 32)
        {
            std::vector<u64> coefficients(size);
            for (std::size_t i = 0; i < left.size(); ++i)
            {
                for (std::size_t j = 0; j < right.size(); ++j)
                {
                    coefficients[i + j] += static_cast<u64>(left[i]) * right[j];
                }
            }
            return carry_digits(size, [&](std::size_t i)
                                {
                                    return coefficients[i];
                                });
        }

        // 係数の上限は min(A,B)*(65536-1)^2。変換長の上限内なら
        // 2法の積より小さく、CRTで整数として一意に復元できる。
        const auto first = NTT<kFirstPrime, 3>{}.convolution_normalized(left, right);
        const auto second = NTT<kSecondPrime, 3>{}.convolution_normalized(left, right);
        return carry_digits(size, [&](std::size_t i)
                            {
                                return restore_coefficient(first[i], second[i]);
                            });
    }

    static BigInteger slice_limbs(const BigInteger &value, std::size_t first, std::size_t count)
    {
        BigInteger result;
        if (first < value.limbs_.size())
        {
            const auto length = std::min(count, value.limbs_.size() - first);
            const auto begin = value.limbs_.begin() + static_cast<std::ptrdiff_t>(first);
            result.limbs_.assign(begin, begin + static_cast<std::ptrdiff_t>(length));
            result.trim();
        }
        return result;
    }

    static BigInteger join_limbs(const BigInteger &high, const BigInteger &low, std::size_t offset)
    {
        if (high.limbs_.empty())
        {
            return low;
        }
        BigInteger result;
        result.limbs_.resize(offset + high.limbs_.size());
        std::copy(low.limbs_.begin(), low.limbs_.end(), result.limbs_.begin());
        std::copy(high.limbs_.begin(), high.limbs_.end(), result.limbs_.begin() + static_cast<std::ptrdiff_t>(offset));
        return result;
    }

    static void scale_limbs(BigInteger &value, u32 factor)
    {
        u64 carry = 0;
        for (auto &limb : value.limbs_)
        {
            const auto scaled = static_cast<u64>(limb) * factor + carry;
            limb = static_cast<u32>(scaled % kBase);
            carry = scaled / kBase;
        }
        if (carry != 0)
        {
            value.limbs_.push_back(static_cast<u32>(carry));
        }
    }

    // 除数の最上位桁はkBase/2以上。上位2桁で商を見積もり、引き過ぎを補正する。
    static std::pair<BigInteger, BigInteger> divide_normalized(const BigInteger &dividend, const BigInteger &divisor)
    {
        BigInteger quotient;
        BigInteger remainder = dividend;
        const auto m = divisor.limbs_.size();
        const auto &b = divisor.limbs_;
        auto &a = remainder.limbs_;
        const auto size = a.size() - m + 1;
        a.push_back(0);
        quotient.limbs_.resize(size);
        for (auto next = size; next != 0; --next)
        {
            const auto offset = next - 1;
            const auto upper = static_cast<u64>(a[offset + m]) * kBase + a[offset + m - 1];
            auto digit = std::min(upper / b[m - 1], u64{kBase - 1});
            auto estimate_remainder = upper - digit * b[m - 1];
            while (digit * b[m - 2] > estimate_remainder * kBase + a[offset + m - 2])
            {
                --digit;
                estimate_remainder += b[m - 1];
            }

            u64 carry = 0;
            for (std::size_t j = 0; j < m; ++j)
            {
                const auto product = digit * b[j] + carry;
                const auto unit = static_cast<u32>(product % kBase);
                const auto borrowed = a[offset + j] < unit;
                a[offset + j] = a[offset + j] + static_cast<u32>(borrowed) * kBase - unit;
                carry = product / kBase + static_cast<u32>(borrowed);
            }
            if (a[offset + m] < carry)
            {
                --digit;
                u32 restored_carry = 0;
                for (std::size_t j = 0; j < m; ++j)
                {
                    const auto sum = a[offset + j] + b[j] + restored_carry;
                    restored_carry = sum >= kBase;
                    a[offset + j] = sum - restored_carry * kBase;
                }
            }
            a[offset + m] = 0;
            quotient.limbs_[offset] = static_cast<u32>(digit);
        }
        quotient.trim();
        remainder.trim();
        return {std::move(quotient), std::move(remainder)};
    }

    // Burnikel–Ziegler: 正規化済みの除数2ブロックで、被除数3ブロックを割る。
    static std::pair<BigInteger, BigInteger> divide_three_by_two(
        const BigInteger &dividend, const BigInteger &divisor,
        const BigInteger &high, const BigInteger &low, std::size_t block)
    {
        BigInteger quotient;
        BigInteger remainder;
        const auto high_dividend = slice_limbs(dividend, 2 * block, block);
        if (magnitude_order(high_dividend, high) < 0)
        {
            auto [q, r] = divide_two_by_one(slice_limbs(dividend, block, 2 * block), high);
            quotient = std::move(q);
            remainder = std::move(r);
        }
        else
        {
            quotient.limbs_.assign(block, kBase - 1);
            remainder = slice_limbs(dividend, block, block) + high;
        }
        remainder = join_limbs(remainder, slice_limbs(dividend, 0, block), block) - quotient * low;
        while (remainder.negative_)
        {
            quotient -= BigInteger(1);
            remainder += divisor;
        }
        return {std::move(quotient), std::move(remainder)};
    }

    static std::pair<BigInteger, BigInteger> divide_two_by_one(const BigInteger &dividend, const BigInteger &divisor)
    {
        if (dividend.limbs_.size() < divisor.limbs_.size())
        {
            return {BigInteger{}, dividend};
        }
        if (divisor.limbs_.size() <= kDivisionLeafSize)
        {
            return divide_normalized(dividend, divisor);
        }

        const auto block = divisor.limbs_.size() / 2;
        const auto high = slice_limbs(divisor, block, block);
        const auto low = slice_limbs(divisor, 0, block);
        const auto [leading, kept] = divide_three_by_two(
            slice_limbs(dividend, block, 3 * block), divisor, high, low, block);
        auto [trailing, remainder] = divide_three_by_two(
            join_limbs(kept, slice_limbs(dividend, 0, block), block), divisor, high, low, block);
        return {join_limbs(leading, trailing, block), std::move(remainder)};
    }

    static std::pair<BigInteger, BigInteger> divide_blocks(const BigInteger &dividend, const BigInteger &divisor)
    {
        const auto block = divisor.limbs_.size();
        const auto chunks = (dividend.limbs_.size() - 1) / block + 1;
        BigInteger quotient;
        quotient.limbs_.resize(dividend.limbs_.size());
        BigInteger remainder;
        for (auto index = chunks; index != 0; --index)
        {
            const auto first = (index - 1) * block;
            const auto active = join_limbs(remainder, slice_limbs(dividend, first, block), block);
            auto [digits, kept] = divide_two_by_one(active, divisor);
            std::copy(digits.limbs_.begin(), digits.limbs_.end(),
                      quotient.limbs_.begin() + static_cast<std::ptrdiff_t>(first));
            remainder = std::move(kept);
        }
        quotient.trim();
        return {std::move(quotient), std::move(remainder)};
    }

public:
    BigInteger(i64 value = 0) : negative_(value < 0)
    {
        const auto bits = static_cast<u64>(value);
        for (auto magnitude = negative_ ? u64{0} - bits : bits; magnitude > 0; magnitude /= kBase)
        {
            limbs_.push_back(static_cast<u32>(magnitude % kBase));
        }
    }

    explicit BigInteger(std::string_view text)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (text.empty())
        {
            throw std::invalid_argument("BigInteger: empty string");
        }
#endif
        negative_ = text.front() == '-';
        const std::size_t first = negative_ || text.front() == '+' ? 1 : 0;
#ifndef SKIP_RUNTIME_CHECKS
        if (first == text.size())
        {
            throw std::invalid_argument("BigInteger: digits required");
        }
#endif
        limbs_.reserve((text.size() - first) / kBlockSize + 1);
        for (auto end = text.size(); end > first;)
        {
            const auto start = end - first < kBlockSize ? first : end - kBlockSize;
            u32 value = 0;
            for (auto i = start; i < end; ++i)
            {
                const auto digit = parse_digit(text[i]);
#ifndef SKIP_RUNTIME_CHECKS
                if (digit >= Radix)
                {
                    throw std::invalid_argument("BigInteger: invalid digit");
                }
#endif
                value = value * Radix + digit;
            }
            limbs_.push_back(value);
            end = start;
        }
        trim();
    }

    [[nodiscard]] int sign() const noexcept
    {
        return limbs_.empty() ? 0 : (negative_ ? -1 : 1);
    }

    [[nodiscard]] std::string to_string() const
    {
        if (limbs_.empty())
        {
            return "0";
        }
        std::string text;
        text.reserve(kBlockSize * limbs_.size() + static_cast<std::size_t>(negative_));
        if (negative_)
        {
            text.push_back('-');
        }
        for (std::size_t i = limbs_.size(); i != 0; --i)
        {
            std::array<char, kBlockSize> block{};
            auto value = limbs_[i - 1];
            for (auto it = block.rbegin(); it != block.rend(); ++it)
            {
                *it = kDigits[value % Radix];
                value /= Radix;
            }
            auto first = block.begin();
            if (i == limbs_.size())
            {
                while (*first == '0')
                {
                    ++first;
                }
            }
            text.append(first, block.end());
        }
        return text;
    }

    [[nodiscard]] BigInteger operator-() const
    {
        auto result = *this;
        result.negative_ = !result.limbs_.empty() && !result.negative_;
        return result;
    }

    friend std::strong_ordering operator<=>(const BigInteger &left, const BigInteger &right) noexcept
    {
        const auto left_sign = left.sign();
        const auto right_sign = right.sign();
        if (left_sign != right_sign)
        {
            return left_sign <=> right_sign;
        }
        return (left_sign * magnitude_order(left, right)) <=> 0;
    }

    friend bool operator==(const BigInteger &left, const BigInteger &right) noexcept
    {
        return (left <=> right) == std::strong_ordering::equal;
    }

    friend BigInteger operator+(const BigInteger &left, const BigInteger &right)
    {
        return add_or_subtract(left, right, false);
    }

    friend BigInteger operator-(const BigInteger &left, const BigInteger &right)
    {
        return add_or_subtract(left, right, true);
    }

    friend BigInteger operator*(const BigInteger &left, const BigInteger &right)
    {
        BigInteger result;
        if (left.limbs_.empty() || right.limbs_.empty())
        {
            return result;
        }
        result.limbs_ = multiply_magnitudes(left.limbs_, right.limbs_);
        result.negative_ = left.negative_ != right.negative_;
        result.trim();
        return result;
    }

    BigInteger &operator+=(const BigInteger &other)
    {
        return *this = *this + other;
    }

    BigInteger &operator-=(const BigInteger &other)
    {
        return *this = *this - other;
    }

    BigInteger &operator*=(const BigInteger &other)
    {
        return *this = *this * other;
    }

    [[nodiscard]] std::pair<BigInteger, i64> divmod(u32 divisor) const
    {
        if (divisor == 0)
        {
            throw std::domain_error("BigInteger: division by zero");
        }
        auto result = *this;
        u64 remainder = 0;
        for (std::size_t i = limbs_.size(); i != 0; --i)
        {
            const auto value = remainder * kBase + limbs_[i - 1];
            result.limbs_[i - 1] = static_cast<u32>(value / divisor);
            remainder = value % divisor;
        }
        result.trim();
        return {std::move(result), static_cast<i64>(remainder) * sign()};
    }

    [[nodiscard]] std::pair<BigInteger, BigInteger> divmod(const BigInteger &divisor) const
    {
        if (divisor.limbs_.empty())
        {
            throw std::domain_error("BigInteger: division by zero");
        }
        if (magnitude_order(*this, divisor) < 0)
        {
            return {BigInteger{}, *this};
        }
        const auto size = divisor.limbs_.size();
        if (size == 1)
        {
            auto [quotient, remainder] = divmod(divisor.limbs_.front());
            quotient.negative_ = negative_ != divisor.negative_;
            return {std::move(quotient), BigInteger(remainder)};
        }

        const bool recursive = size > kDivisionLeafSize && limbs_.size() - size >= 16;
        const auto padding = recursive ? std::bit_ceil(size) - size : 0;
        auto dividend = join_limbs(*this, BigInteger{}, padding);
        auto normalized = join_limbs(divisor, BigInteger{}, padding);
        const auto factor = kBase / (normalized.limbs_.back() + 1);
        if (factor != 1)
        {
            scale_limbs(dividend, factor);
            scale_limbs(normalized, factor);
        }
        auto [quotient, remainder] = recursive ? divide_blocks(dividend, normalized)
                                               : divide_normalized(dividend, normalized);
        remainder = slice_limbs(remainder, padding, remainder.limbs_.size());
        if (factor != 1)
        {
            remainder = remainder.divmod(factor).first;
        }
        quotient.negative_ = negative_ != divisor.negative_;
        remainder.negative_ = !remainder.limbs_.empty() && negative_;
        return {std::move(quotient), std::move(remainder)};
    }

    friend BigInteger operator/(const BigInteger &left, const BigInteger &right)
    {
        return left.divmod(right).first;
    }

    friend BigInteger operator%(const BigInteger &left, const BigInteger &right)
    {
        return left.divmod(right).second;
    }

    BigInteger &operator/=(const BigInteger &other)
    {
        return *this = *this / other;
    }

    BigInteger &operator%=(const BigInteger &other)
    {
        return *this = *this % other;
    }
};
#endif
