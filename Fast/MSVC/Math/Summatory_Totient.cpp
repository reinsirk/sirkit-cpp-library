// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Summatory_Totient.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/summatory_totient.md
//
// API一覧
// summatory_totient(n,mod=998244353): φ(1)+…+φ(n)をmodで割った余り。n=0は0、mod>=1。
// φの線形篩と、同じfloor(n/i)をまとめた漸化式。O(n^(2/3))時間・O(n^(2/3))領域。
// 篩の添字は32ビットまで。必要な配列を確保できない場合は標準の例外。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUMMATORY_TOTIENT_DEFINED
#define SIRKIT_SUMMATORY_TOTIENT_DEFINED
namespace detail
{
inline std::uint64_t summatory_totient_limit(std::uint64_t n)
{
    std::uint64_t low = 0, high = std::uint64_t{1} << 22;
    while (high - low > 1)
    {
        const auto mid = (low + high) / 2;
        if (mid <= n / (mid * mid))
        {
            low = mid;
        }
        else
        {
            high = mid;
        }
    }
    const auto ceil_cube_root = low + (low * low * low != n);
    return std::min(n, ceil_cube_root * ceil_cube_root);
}

inline std::vector<std::uint32_t> summatory_totient_prefix(std::uint64_t limit, std::uint32_t mod)
{
    std::vector<std::uint32_t> prefix;
    const auto max_limit = std::min<std::uint64_t>(std::numeric_limits<std::uint32_t>::max(), prefix.max_size() - 1);
    if (limit > max_limit)
    {
        throw std::length_error("summatory_totient: table too large");
    }
    prefix.resize(static_cast<std::size_t>(limit) + 1);
    prefix[1] = 1;
    std::vector<std::uint32_t> primes;
    for (std::uint64_t i = 2; i <= limit; ++i)
    {
        if (prefix[i] == 0)
        {
            prefix[i] = static_cast<std::uint32_t>(i - 1);
            primes.push_back(static_cast<std::uint32_t>(i));
        }
        // 最小素因数は primes に含まれる。積の上限か最小素因数に到達したら止まる。
        for (std::size_t j = 0;; ++j)
        {
            const auto prime = primes[j];
            const auto next = i * prime;
            if (next > limit)
            {
                break;
            }
            if (i % prime == 0)
            {
                prefix[next] = prefix[i] * prime;
                break;
            }
            prefix[next] = prefix[i] * (prime - 1);
        }
    }
    for (std::uint64_t i = 1; i <= limit; ++i)
    {
        prefix[i] = static_cast<std::uint32_t>((std::uint64_t{prefix[i]} + prefix[i - 1]) % mod);
    }
    return prefix;
}

inline std::uint64_t summatory_totient_triangle(std::uint64_t x, std::uint32_t mod)
{
    // modが偶数でもよいように、偶数側を先に整数として2で割る。
    const auto half = (x / 2 + (x & 1)) % mod;
    const auto odd = (x + ((x & 1) == 0)) % mod;
    return half * odd % mod;
}
} // namespace detail

[[nodiscard]] inline std::uint32_t summatory_totient(std::uint64_t n, std::uint32_t mod = 998244353)
{
    if (mod == 0)
    {
        throw std::invalid_argument("summatory_totient: modulus is zero");
    }
    if (n == 0 || mod == 1)
    {
        return 0;
    }

    const auto limit = detail::summatory_totient_limit(n);
    const auto prefix = detail::summatory_totient_prefix(limit, mod);
    if (n == limit)
    {
        return prefix[n];
    }
    const auto count = n / (limit + 1);
    std::vector<std::uint32_t> large(static_cast<std::size_t>(count + 1));
    for (auto index = count; index != 0; --index)
    {
        const auto x = n / index;
        auto sum = detail::summatory_totient_triangle(x, mod);
        // sum_{i=1}^x Φ(floor(x/i)) = 1 + 2 + ... + x。
        for (std::uint64_t left = 2; left <= x;)
        {
            const auto quotient = x / left;
            const auto right = x / quotient;
            const auto value = quotient <= limit ? prefix[quotient] : large[n / quotient];
            const auto term = (right - left + 1) % mod * value % mod;
            sum = (sum >= term ? sum : sum + mod) - term;
            left = right + 1;
        }
        large[index] = static_cast<std::uint32_t>(sum);
    }
    return large[1];
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
