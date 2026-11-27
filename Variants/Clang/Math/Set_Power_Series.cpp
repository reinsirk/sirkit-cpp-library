// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Set_Power_Series.cpp
// 部分集合畳み込みの参考: Björklund et al., Fourier Meets Möbius: Fast Subset Convolution (2007).
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/set_power_series.md
//
// API一覧
// subset_convolution(a,b): maskごとに、要素を重複なく分ける組の a[sub]*b[mask^sub] の和を返す。
// set_series_exp(f): 定数項（添字0）が0の集合冪級数を指数化する。
// set_series_log(f): 定数項（添字0）が1の集合冪級数の対数を取る。
// いずれも法998244353。同じ長さ2^nのuint32_t列を渡し、nは0..20。uint32_t全域を法で正規化する。
// 要素は集合のビットマスクで添字を付ける。nを全集合の要素数とし、各O((n+1)^2*2^n)時間・O((n+1)*2^n)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SET_POWER_SERIES_DEFINED
#define SIRKIT_SET_POWER_SERIES_DEFINED
namespace detail
{
inline constexpr std::uint32_t kSetSeriesMod = 998244353;
inline constexpr std::size_t kSetSeriesMaxSize = std::size_t{1} << 20;

inline void CheckSetSeriesSize(std::size_t size)
{
    if (!std::has_single_bit(size) || size > kSetSeriesMaxSize)
    {
        throw std::length_error("set power series: expected a power of two up to 2^20");
    }
}

inline std::vector<std::uint32_t> SetSubsetProduct(std::span<const std::uint32_t> a,
                                                   std::span<const std::uint32_t> b)
{
    const std::size_t n = static_cast<std::size_t>(std::countr_zero(a.size()));
    const std::size_t width = n + 1;
    std::vector<std::uint8_t> ranks(a.size());
    std::vector<std::uint32_t> left(a.size() * width), right(left.size());
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        ranks[i] = static_cast<std::uint8_t>(std::popcount(i));
        left[i * width + ranks[i]] = a[i] % kSetSeriesMod;
        right[i * width + ranks[i]] = b[i] % kSetSeriesMod;
    }

    // 集合の大きさ別に両列のゼータ変換を計算する。
    for (std::size_t step = 1; step < a.size(); step *= 2)
    {
        for (std::size_t block = 0; block < a.size(); block += step * 2)
        {
            for (std::size_t i = block; i < block + step; ++i)
            {
                const auto src = i * width;
                const auto dest = src + step * width;
                for (std::size_t d = 0; d <= ranks[i]; ++d)
                {
                    const auto x = left[dest + d] + left[src + d];
                    const auto y = right[dest + d] + right[src + d];
                    left[dest + d] = x >= kSetSeriesMod ? x - kSetSeriesMod : x;
                    right[dest + d] = y >= kSetSeriesMod ? y - kSetSeriesMod : y;
                }
            }
        }
    }

    // 逆変換で使う次数のみ掛ける。大きい次数から上書きすると、入力を別に保存せずに済む。
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        const std::size_t rank = ranks[i];
        auto *x = left.data() + i * width;
        const auto *y = right.data() + i * width;
        for (std::size_t d = std::min(n, rank * 2) + 1; d > rank;)
        {
            --d;
            const auto first = d - rank;
            std::uint64_t sum = 0;
            // 法未満の積は18項までuint64_tに収まる。長い区間だけ途中で剰余を取る。
            auto j = first;
            if (rank - first >= 17)
            {
                for (; j < first + 16; ++j)
                {
                    sum += std::uint64_t{x[j]} * y[d - j];
                }
                sum %= kSetSeriesMod;
            }
            for (; j <= rank; ++j)
            {
                sum += std::uint64_t{x[j]} * y[d - j];
            }
            x[d] = static_cast<std::uint32_t>(sum % kSetSeriesMod);
        }
    }

    for (std::size_t step = 1; step < a.size(); step *= 2)
    {
        for (std::size_t block = 0; block < a.size(); block += step * 2)
        {
            for (std::size_t i = block; i < block + step; ++i)
            {
                const auto src = i * width;
                const auto dest = src + step * width;
                for (std::size_t d = static_cast<std::size_t>(ranks[i]) + 1; d <= n; ++d)
                {
                    const auto x = left[dest + d];
                    const auto y = left[src + d];
                    left[dest + d] = x >= y ? x - y : x + kSetSeriesMod - y;
                }
            }
        }
    }
    std::vector<std::uint32_t> result(a.size());
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        result[i] = left[i * width + ranks[i]];
    }
    return result;
}
}

inline std::vector<std::uint32_t> subset_convolution(std::span<const std::uint32_t> a,
                                                     std::span<const std::uint32_t> b)
{
    detail::CheckSetSeriesSize(a.size());
#ifndef SKIP_RUNTIME_CHECKS
    if (a.size() != b.size())
    {
        throw std::invalid_argument("subset_convolution: size mismatch");
    }
#endif
    return detail::SetSubsetProduct(a, b);
}

inline std::vector<std::uint32_t> set_series_exp(std::span<const std::uint32_t> f)
{
    detail::CheckSetSeriesSize(f.size());
    if (f[0] % detail::kSetSeriesMod != 0)
    {
        throw std::domain_error("set_series_exp: constant coefficient must be zero modulo 998244353");
    }
    std::vector<std::uint32_t> result(f.size());
    result[0] = 1;
    // x^2=0ならexp(f0 + x f1)=exp(f0)(1 + x f1)。変数を1個ずつ追加する。
    for (std::size_t length = 1; length < f.size(); length *= 2)
    {
        const auto high = detail::SetSubsetProduct(std::span<const std::uint32_t>(result).first(length),
                                                   f.subspan(length, length));
        std::copy(high.begin(), high.end(), result.begin() + length);
    }
    return result;
}

inline std::vector<std::uint32_t> set_series_log(std::span<const std::uint32_t> f)
{
    detail::CheckSetSeriesSize(f.size());
    if (f[0] % detail::kSetSeriesMod != 1)
    {
        throw std::domain_error("set_series_log: constant coefficient must be one modulo 998244353");
    }
    std::vector<std::uint32_t> result(f.size());
    std::vector<std::uint32_t> inverse{1};
    inverse.reserve(f.size() / 2);
    // log(f0 + x f1)=log(f0) + x f1/f0。逆元の追加分は -x f1/f0^2。
    for (std::size_t length = 1; length < f.size(); length *= 2)
    {
        const auto high = detail::SetSubsetProduct(inverse, f.subspan(length, length));
        std::copy(high.begin(), high.end(), result.begin() + length);
        if (length * 2 < f.size())
        {
            const auto next = detail::SetSubsetProduct(inverse, high);
            for (const auto x : next)
            {
                inverse.push_back(x == 0 ? 0 : detail::kSetSeriesMod - x);
            }
        }
    }
    return result;
}
#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
