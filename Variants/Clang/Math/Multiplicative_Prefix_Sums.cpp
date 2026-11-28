// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Multiplicative_Prefix_Sums.cpp
// 参考（Lucy DPと簡略化したMin_25篩）: https://atcoder.jp/contests/abc370/editorial/10906
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/multiplicative_prefix_sums.md
//
// API一覧
// [MultiplicativePrefixSums]
// MultiplicativePrefixSums(n): n>=1を固定し、sqrt(n)以下の素数を準備する。O(sqrt(n) log log(n+2))時間。
// size(), quotient(index): 相異なるfloor(n/i)の個数、昇順0始まりの座標。各O(1)時間。
// prime_prefix_sums(weight, prefix): 完全乗法的weightの素数に限った累積和。prefix(x)は1..xのweightの総和。
// prefix_sums(prime_sums, prime_power): 素数での累積和とf(p^e)を受け取り、f(1)=1の乗法的関数の累積和を返す。
// prefix_sum(prime_sums, prime_power): n以下の総和1個だけを、最小素因数の列挙で求める。
// 各コールバックとTの演算をO(1)とする。prime_prefix_sumsとprefix_sumsは各O(n^(3/4)/log(n+2))時間。
// prefix_sumは探索する素因数列の個数に比例し、最悪O(n)時間。すべてO(sqrt(n))作業領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MULTIPLICATIVE_PREFIX_SUMS_DEFINED
#define SIRKIT_MULTIPLICATIVE_PREFIX_SUMS_DEFINED
class MultiplicativePrefixSums
{
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    u64 n_;
    u64 small_;
    u64 large_;
    std::vector<u32> primes_;

    static u64 square_root(u64 value) noexcept
    {
        u64 upper = u64{1} << ((std::bit_width(value) + 1) / 2);
        for (u64 next = (upper + value / upper) / 2; next < upper; next = (upper + value / upper) / 2)
        {
            upper = next;
        }
        return upper;
    }

    [[nodiscard]] u64 coordinate(u64 index) const noexcept
    {
        return index < small_ ? index + 1 : n_ / (size() - index);
    }

    [[nodiscard]] u64 rank(u64 value) const noexcept
    {
        return value <= small_ ? value - 1 : size() - n_ / value;
    }

    // v>=p^2を降順に処理する。v/pも必ず同じ商の座標に含まれる。
    template <class Update>
    void process(u64 p, const Update &update) const
    {
        const auto square = p * p;
        const auto end = std::min(large_, n_ / square);
        const auto large_end = std::min(end, large_ / p);
        for (u64 d = 1; d <= large_end; ++d)
        {
            const auto product = d * p;
            update(size() - d, size() - product, n_ / product);
        }
        for (u64 d = large_end + 1; d <= end; ++d)
        {
            const auto divided = n_ / (d * p);
            update(size() - d, divided - 1, divided);
        }
        for (u64 value = small_; value >= square; --value)
        {
            const auto divided = static_cast<u32>(value) / static_cast<u32>(p);
            update(value - 1, divided - 1, divided);
        }
    }

    static u64 compact_quotient(u64 dividend, u64 divisor) noexcept
    {
        // 呼び出し時は0<divisor<=dividend。被除数が32ビットなら除数も32ビットに収まる。
        if (dividend <= std::numeric_limits<u32>::max())
        {
            return static_cast<u32>(dividend) / static_cast<u32>(divisor);
        }
        return dividend / divisor;
    }

    void check_prime_table_size(std::size_t length) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (length != size())
        {
            throw std::invalid_argument("MultiplicativePrefixSums: prime prefix table size");
        }
#else
        (void)length;
#endif
    }

    template <class T>
    struct CachedPrime
    {
        u64 prime;
        u64 square;
        T prefix;
        T first_power;
        T second_power;
    };

    [[nodiscard]] u64 divisor_rank(u64 value, u64 denominator) const noexcept
    {
        return denominator <= large_ ? size() - denominator : value - 1;
    }

    template <class T, class PrimePower>
    [[nodiscard]] T sum_without_one(u64 value, u64 denominator, std::size_t first,
                                    const std::vector<CachedPrime<T>> &rows, const std::vector<T> &prime_sums,
                                    const PrimePower &prime_power) const
    {
        const auto below = first == 0 ? T{} : rows[first - 1].prefix;
        T answer = prime_sums[divisor_rank(value, denominator)] - below;
        for (auto i = first; i < rows.size(); ++i)
        {
            const auto &row = rows[i];
            const auto prime = row.prime;
            if (row.square > value)
            {
                break;
            }
            const auto divided = compact_quotient(value, prime);
            const auto next_denominator = denominator * prime;
            if (divided < row.square)
            {
                answer += row.first_power * (prime_sums[divisor_rank(divided, next_denominator)] - row.prefix) + row.second_power;
                continue;
            }

            answer += row.first_power * sum_without_one(divided, next_denominator, i + 1, rows, prime_sums, prime_power) + row.second_power;
            auto current = compact_quotient(divided, prime);
            auto divisor = next_denominator * prime;
            auto power = row.second_power;
            for (unsigned exponent = 3; current >= prime; ++exponent)
            {
                const auto next_power = prime_power(prime, exponent);
                if (current < row.square)
                {
                    answer += power * (prime_sums[divisor_rank(current, divisor)] - row.prefix) + next_power;
                }
                else
                {
                    answer += power * sum_without_one(current, divisor, i + 1, rows, prime_sums, prime_power) + next_power;
                }
                current = compact_quotient(current, prime);
                divisor *= prime;
                power = next_power;
            }
        }
        return answer;
    }

public:
    explicit MultiplicativePrefixSums(u64 n) : n_(n)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (n == 0)
        {
            throw std::invalid_argument("MultiplicativePrefixSums: positive n required");
        }
#endif
        small_ = square_root(n);
        large_ = n / (small_ + 1);
        if constexpr (sizeof(std::size_t) < sizeof(u64))
        {
            if (size() >= std::numeric_limits<std::size_t>::max())
            {
                throw std::length_error("MultiplicativePrefixSums: table size");
            }
        }
        std::vector<bool> composite(static_cast<std::size_t>(small_) + 1);
        for (u64 p = 2; p <= small_; ++p)
        {
            if (composite[p])
            {
                continue;
            }
            primes_.push_back(static_cast<u32>(p));
            for (u64 multiple = p * p; multiple <= small_; multiple += p)
            {
                composite[multiple] = true;
            }
        }
    }

    [[nodiscard]] u64 size() const noexcept
    {
        return small_ + large_;
    }

    [[nodiscard]] u64 quotient(u64 index) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (index >= size())
        {
            throw std::out_of_range("MultiplicativePrefixSums: quotient index");
        }
#endif
        return coordinate(index);
    }

    template <class Weight, class Prefix>
    [[nodiscard]] auto prime_prefix_sums(const Weight &weight, const Prefix &prefix) const
    {
        using T = std::remove_cvref_t<decltype(prefix(u64{1}))>;
        const auto total = size();
        std::vector<T> values(static_cast<std::size_t>(total));
        for (u64 index = 0; index < total; ++index)
        {
            values[index] = prefix(coordinate(index)) - T{1};
        }
        for (const u64 prime : primes_)
        {
            const auto lower_primes = values[prime - 2];
            const auto prime_weight = weight(prime);
            const auto end = std::min(large_, compact_quotient(n_, prime * prime));
            const auto direct = std::min(end, large_ / prime);
            for (u64 d = 1, source = prime; d <= direct; ++d, source += prime)
            {
                values[total - d] -= prime_weight * (values[total - source] - lower_primes);
            }

            const auto numerator = compact_quotient(n_, prime);
            const auto split = std::min(end, 2 * square_root(numerator));
            auto d = direct + 1;
            for (; d <= split; ++d)
            {
                const auto source = compact_quotient(numerator, d);
                values[total - d] -= prime_weight * (values[source - 1] - lower_primes);
            }
            while (d <= end)
            {
                const auto source = compact_quotient(numerator, d);
                const auto last = std::min(end, compact_quotient(numerator, source)); // 同じ商になる範囲の右端。
                const auto correction = prime_weight * (values[source - 1] - lower_primes);
                for (; d <= last; ++d)
                {
                    values[total - d] -= correction;
                }
            }

            auto remaining = small_;
            for (u64 source = small_ / prime; source >= prime; --source)
            {
                const auto first = source * prime;
                const auto correction = prime_weight * (values[source - 1] - lower_primes);
                for (; remaining >= first; --remaining)
                {
                    values[remaining - 1] -= correction;
                }
            }
        }
        return values;
    }

    template <class T, class PrimePower>
    [[nodiscard]] T prefix_sum(const std::vector<T> &prime_sums, const PrimePower &prime_power) const
    {
        check_prime_table_size(prime_sums.size());
        std::vector<CachedPrime<T>> rows;
        rows.reserve(primes_.size());
        for (const u64 prime : primes_)
        {
            rows.push_back({prime, prime * prime, prime_sums[prime - 1], prime_power(prime, 1), prime_power(prime, 2)});
        }
        return T{1} + sum_without_one(n_, 1, 0, rows, prime_sums, prime_power);
    }

    template <class T, class PrimePower>
    [[nodiscard]] std::vector<T> prefix_sums(std::vector<T> prime_sums, const PrimePower &prime_power) const
    {
        check_prime_table_size(prime_sums.size());
        for (auto it = primes_.rbegin(); it != primes_.rend(); ++it)
        {
            const u64 prime = *it;
            const auto prime_square = prime * prime;
            const auto lower_primes = prime_sums[prime - 1];
            std::array<T, 64> powers{};
            for (u64 left = n_, exponent = 1; left >= prime; left /= prime, ++exponent)
            {
                powers[exponent] = prime_power(prime, static_cast<unsigned>(exponent));
            }
            process(prime, [&](u64 target, u64 divided_rank, u64 divided)
                    {
                        // 最小素因数がprimeの合成数を、primeの指数で分けて戻す。
                        T value = powers[1] * (prime_sums[divided_rank] - lower_primes) + powers[2];
                        for (unsigned e = 2; divided >= prime_square; ++e)
                        {
                            divided /= prime;
                            value += powers[e] * (prime_sums[rank(divided)] - lower_primes) + powers[e + 1];
                        }
                        prime_sums[target] += value;
                    });
        }
        for (auto &value : prime_sums)
        {
            value += T{1};
        }
        return prime_sums;
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
