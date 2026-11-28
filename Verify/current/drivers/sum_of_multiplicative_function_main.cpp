// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace
{
class Residue
{
    static constexpr std::uint32_t kModulus = 469762049;
    std::uint32_t value_;

public:
    Residue(std::uint64_t value = 0) : value_(static_cast<std::uint32_t>(value % kModulus))
    {
    }

    Residue &operator+=(Residue other)
    {
        value_ += other.value_;
        value_ -= value_ >= kModulus ? kModulus : 0;
        return *this;
    }

    Residue &operator-=(Residue other)
    {
        value_ += value_ < other.value_ ? kModulus : 0;
        value_ -= other.value_;
        return *this;
    }

    friend Residue operator+(Residue left, Residue right)
    {
        return left += right;
    }

    friend Residue operator-(Residue left, Residue right)
    {
        return left -= right;
    }

    friend Residue operator*(Residue left, Residue right)
    {
        return Residue{static_cast<std::uint64_t>(left.value_) * right.value_};
    }

    [[nodiscard]] std::uint32_t value() const
    {
        return value_;
    }
};

struct PrimeSums
{
    Residue count;
    Residue sum;

    explicit PrimeSums(std::uint64_t value = 0) : count(value), sum(value)
    {
    }

    PrimeSums(Residue count_value, Residue sum_value) : count(count_value), sum(sum_value)
    {
    }

    PrimeSums &operator-=(PrimeSums other)
    {
        count -= other.count;
        sum -= other.sum;
        return *this;
    }

    friend PrimeSums operator-(PrimeSums left, PrimeSums right)
    {
        return left -= right;
    }

    friend PrimeSums operator*(PrimeSums left, PrimeSums right)
    {
        return {left.count * right.count, left.sum * right.sum};
    }
};
}

int main()
{
    int count{};
    verify_in >> count;
    while (count-- > 0)
    {
        std::uint64_t n{};
        std::uint32_t a_raw{}, b_raw{};
        verify_in >> n >> a_raw >> b_raw;
        const Residue a = a_raw, b = b_raw;
        const sirkit::MultiplicativePrefixSums sieve(n);
        std::vector<Residue> values;
        {
            const auto prime_sums = sieve.prime_prefix_sums([](std::uint64_t p)
                                                            {
                                                                return PrimeSums{Residue{1}, Residue{p}};
                                                            },
                                                            [](std::uint64_t x)
                                                            {
                                                                const auto sum = x % 2 != 0 ? Residue{x} * Residue{x / 2 + 1} : Residue{x / 2} * Residue{x + 1};
                                                                return PrimeSums{Residue{x}, sum};
                                                            });
            values.reserve(prime_sums.size());
            for (const auto &entry : prime_sums)
            {
                values.emplace_back(a * entry.count + b * entry.sum);
            }
        }
        const auto answer = sieve.prefix_sum(values, [&](std::uint64_t prime, unsigned exponent)
                                             {
                                                 return a * Residue{exponent} + b * Residue{prime};
                                             });
        verify_out << answer.value() << '\n';
    }
}
