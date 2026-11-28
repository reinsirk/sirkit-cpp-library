// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/small_prime_stirling.md
//
// API一覧
// [SmallPrimeStirlingNumbers]
// first_kind(p): 符号付き第1種を法pで用意する。下降階乗x(x-1)...(x-n+1)の係数。
// second_kind(p): 第2種を法pで用意する。n要素をk個の空でない集合に分ける個数。
// pは5000以下の素数。各構築はO(p^2)時間・領域。
// modulus(): 法pをO(1)で返す。get(n,k): n,kはuint64_t、O(log_p(n+1)+1)時間。
// get(0,0)=1、k>nなら0。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SMALL_PRIME_STIRLING_DEFINED
#define SIRKIT_SMALL_PRIME_STIRLING_DEFINED
class SmallPrimeStirlingNumbers
{
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    const u32 prime_;
    const bool first_;
    std::vector<std::uint16_t> factorial_;
    std::vector<std::uint16_t> inverse_factorial_;
    std::vector<std::uint16_t> table_;

    static u32 checked_prime(u32 prime)
    {
        if (prime < 2 || prime > 5000)
        {
            throw std::invalid_argument("SmallPrimeStirlingNumbers: expected a prime up to 5000");
        }
        for (u32 divisor = 2; divisor * divisor <= prime; ++divisor)
        {
            if (prime % divisor == 0)
            {
                throw std::invalid_argument("SmallPrimeStirlingNumbers: expected a prime");
            }
        }
        return prime;
    }

    static std::size_t first_offset(u32 n) noexcept
    {
        return std::size_t{n} * (n + 1) / 2;
    }

    [[nodiscard]] std::size_t second_offset(u32 k) const noexcept
    {
        return std::size_t{k} * (prime_ - 1);
    }

    SmallPrimeStirlingNumbers(u32 prime, bool first)
        : prime_(checked_prime(prime)), first_(first), factorial_(prime_, 1), inverse_factorial_(prime_, 1)
    {
        for (u32 i = 1; i < prime_; ++i)
        {
            factorial_[i] = static_cast<std::uint16_t>(factorial_[i - 1] * i % prime_);
        }
        // (p-1)! = -1 なので、その逆元も -1。
        inverse_factorial_.back() = static_cast<std::uint16_t>(prime_ - 1);
        for (u32 i = prime_ - 1; i > 0; --i)
        {
            inverse_factorial_[i - 1] = static_cast<std::uint16_t>(inverse_factorial_[i] * i % prime_);
        }
        if (first_)
        {
            table_.resize(first_offset(prime_));
            table_[0] = 1;
            for (u32 n = 1; n < prime_; ++n)
            {
                const auto current = first_offset(n);
                const auto previous = first_offset(n - 1);
                for (u32 k = 1; k < n; ++k)
                {
                    const u32 value = table_[previous + k - 1] + (prime_ + 1 - n) * table_[previous + k];
                    table_[current + k] = static_cast<std::uint16_t>(value % prime_);
                }
                table_[current + n] = 1;
            }
        }
        else
        {
            table_.resize(second_offset(prime_));
            table_[0] = 1;
            for (u32 k = 1; k < prime_; ++k)
            {
                const auto current = second_offset(k);
                const auto previous = second_offset(k - 1);
                table_[current] = 1;
                for (u32 n_minus_k = 1; n_minus_k < prime_ - 1; ++n_minus_k)
                {
                    const u32 value = table_[previous + n_minus_k] + k * table_[current + n_minus_k - 1];
                    table_[current + n_minus_k] = static_cast<std::uint16_t>(value % prime_);
                }
            }
        }
    }

    [[nodiscard]] u32 binomial(u64 n, u64 k) const noexcept
    {
        u32 result = 1;
        for (; k != 0; n /= prime_, k /= prime_)
        {
            const auto nd = static_cast<u32>(n % prime_);
            const auto kd = static_cast<u32>(k % prime_);
            if (kd > nd)
            {
                return 0;
            }
            const auto digit = factorial_[nd] * static_cast<u32>(inverse_factorial_[kd]) % prime_ * inverse_factorial_[nd - kd] % prime_;
            result = result * digit % prime_;
        }
        return result;
    }

    [[nodiscard]] u32 get_first(u64 n, u64 k) const noexcept
    {
        // (x)_n = x^q (x^(p-1)-1)^q (x)_r, n=pq+r。
        const auto blocks = n / prime_;
        if (k < blocks)
        {
            return 0;
        }
        const auto residue = static_cast<u32>(n % prime_);
        const auto exponent = k - blocks;
        const u32 period = prime_ - 1;
        auto tail = static_cast<u32>(exponent % period);
        if (residue != 0 && tail == 0)
        {
            // (x)_rの定数項は0。次数p-1が同じ剰余を持つ。
            tail = period;
        }
        if (tail > residue || tail > exponent)
        {
            return 0;
        }
        // k<=nかつtailが残りの次数以内なので、selectedはblocks以下。
        const auto selected = (exponent - tail) / period;
        const auto value = binomial(blocks, selected) * table_[first_offset(residue) + tail] % prime_;
        if ((blocks - selected) % 2 == 1 && value != 0)
        {
            return prime_ - value;
        }
        return value;
    }

    [[nodiscard]] u32 get_second(u64 n, u64 k) const noexcept
    {
        if (k == 0)
        {
            return n == 0;
        }
        // 1..pの逆数級数の積は1/(1-x^(p-1))。残りr個の係数はr>0なら周期p-1。
        const auto blocks = k / prime_;
        const auto residue = static_cast<u32>(k % prime_);
        const auto steps = (n - k) / (prime_ - 1);
        const auto offset = static_cast<u32>((n - k) % (prime_ - 1));
        if (residue == 0)
        {
            if (offset != 0)
            {
                return 0;
            }
            return binomial(blocks + steps - 1, steps);
        }
        return binomial(blocks + steps, steps) * table_[second_offset(residue) + offset] % prime_;
    }

public:
    [[nodiscard]] static SmallPrimeStirlingNumbers first_kind(u32 prime)
    {
        return SmallPrimeStirlingNumbers(prime, true);
    }

    [[nodiscard]] static SmallPrimeStirlingNumbers second_kind(u32 prime)
    {
        return SmallPrimeStirlingNumbers(prime, false);
    }

    [[nodiscard]] u32 modulus() const noexcept
    {
        return prime_;
    }

    [[nodiscard]] u32 get(u64 n, u64 k) const noexcept
    {
        if (k > n)
        {
            return 0;
        }
        return first_ ? get_first(n, k) : get_second(n, k);
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
