// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Combination/Binomial_Prefix_Sum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/binomial_prefix_sum.md
//
// API一覧
// binomial_prefix_sums<Mod>(queries): (n,k)ごとにC(n,0)+…+C(n,k)をModで割った余りを返す。
// Modは奇素数で既定値は998244353。n<Mod。k>=nなら2^n。答えは入力順。
// 最大nをN、問い合わせ数をQとしてO(N+Q log(Q+1)+N sqrt(Q))時間、O(N+Q)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BINOMIAL_PREFIX_SUMS_DEFINED
#define SIRKIT_BINOMIAL_PREFIX_SUMS_DEFINED
namespace detail
{
template <std::uint32_t Mod>
struct BinomialPrefixArithmetic
{
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    static consteval bool is_odd_prime()
    {
        if (Mod < 3 || Mod % 2 == 0)
        {
            return false;
        }
        for (u32 divisor = 3; static_cast<u64>(divisor) * divisor <= Mod; divisor += 2)
        {
            if (Mod % divisor == 0)
            {
                return false;
            }
        }
        return true;
    }

    static u32 add(u32 left, u32 right) noexcept
    {
        const u64 sum = static_cast<u64>(left) + right;
        return static_cast<u32>(sum >= Mod ? sum - Mod : sum);
    }

    static u32 subtract(u32 left, u32 right) noexcept
    {
        return static_cast<u32>(left >= right ? left - right : static_cast<u64>(left) + Mod - right);
    }

    static u32 multiply(u32 left, u32 right) noexcept
    {
        return static_cast<u32>(static_cast<u64>(left) * right % Mod);
    }

    static u32 invert(u32 base) noexcept
    {
        u32 result = 1;
        for (u32 exponent = Mod - 2; exponent > 0; exponent >>= 1)
        {
            if ((exponent & 1u) != 0)
            {
                result = multiply(result, base);
            }
            base = multiply(base, base);
        }
        return result;
    }

    static u32 halve(u32 value) noexcept
    {
        const auto adjustment = (value & 1u) != 0 ? Mod : 0u;
        return static_cast<u32>((static_cast<u64>(value) + adjustment) / 2);
    }
};
}

template <std::uint32_t Mod = 998244353>
[[nodiscard]] std::vector<std::uint32_t> binomial_prefix_sums(
    std::span<const std::pair<std::uint32_t, std::uint32_t>> queries)
{
    using u32 = std::uint32_t;
    using Arithmetic = detail::BinomialPrefixArithmetic<Mod>;
    static_assert(Arithmetic::is_odd_prime(), "binomial_prefix_sums: odd prime modulus required");
    if (queries.empty())
    {
        return {};
    }
    u32 max_n = 0;
    for (const auto &[n, k] : queries)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (n >= Mod)
        {
            throw std::out_of_range("binomial_prefix_sums: n must be below Mod");
        }
#endif
        max_n = std::max(max_n, n);
    }
    const auto table_size = static_cast<std::size_t>(max_n) + 1;
    std::vector<u32> factorial(table_size, 1), inverse(table_size), powers_of_two(table_size, 1);
    for (std::size_t i = 1; i < table_size; ++i)
    {
        factorial[i] = Arithmetic::multiply(factorial[i - 1], static_cast<u32>(i));
        powers_of_two[i] = Arithmetic::add(powers_of_two[i - 1], powers_of_two[i - 1]);
    }
    inverse[max_n] = Arithmetic::invert(factorial[max_n]);
    for (u32 i = max_n; i > 0; --i)
    {
        inverse[i - 1] = Arithmetic::multiply(inverse[i], i);
    }
    const auto choose = [&](u32 n, u32 k)
    {
        return Arithmetic::multiply(Arithmetic::multiply(factorial[n], inverse[k]), inverse[n - k]);
    };

    struct Query
    {
        u32 n;
        u32 k;
        bool reflected;
        std::size_t input_index;
        std::uint64_t sort_key;
    };
    const auto block_size = std::max(u32{1}, static_cast<u32>(max_n / std::sqrt(static_cast<double>(queries.size()))));
    std::vector<u32> result(queries.size());
    std::vector<Query> ordered;
    ordered.reserve(queries.size());
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        const auto [n, k] = queries[i];
        if (k >= n)
        {
            result[i] = powers_of_two[n];
            continue;
        }
        const bool reflected = k > n - 1 - k;
        const auto bound = reflected ? n - 1 - k : k;
        const auto block = n / block_size;
        const auto order_k = (block & 1u) == 0 ? bound : max_n - bound;
        const auto key = (static_cast<std::uint64_t>(block) << 32) | order_k;
        ordered.push_back({n, bound, reflected, i, key});
    }
    std::sort(ordered.begin(), ordered.end(), [](const Query &left, const Query &right)
              {
                  return left.sort_key < right.sort_key;
              });

    u32 n = 0, k = 0, sum = 1;
    for (const auto &query : ordered)
    {
        // nの増加とkの減少を先に行い、途中でもk<=nを維持する。
        while (n < query.n)
        {
            sum = Arithmetic::subtract(Arithmetic::add(sum, sum), choose(n, k));
            ++n;
        }
        while (k > query.k)
        {
            sum = Arithmetic::subtract(sum, choose(n, k));
            --k;
        }
        while (k < query.k)
        {
            ++k;
            sum = Arithmetic::add(sum, choose(n, k));
        }
        while (n > query.n)
        {
            --n;
            sum = Arithmetic::halve(Arithmetic::add(sum, choose(n, k)));
        }
        result[query.input_index] = query.reflected ? Arithmetic::subtract(powers_of_two[n], sum) : sum;
    }
    return result;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
