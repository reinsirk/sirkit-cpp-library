// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Chromatic_Polynomial.cpp
// 参考（独立集合の分割による彩色数え上げ）: https://epubs.siam.org/doi/10.1137/070683933
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/chromatic_polynomial.md
//
// API一覧
// chromatic_polynomial(n, edges): 無向グラフの彩色多項式を法998244353で返す。定数項からn次の順。
// 同色の頂点を隣接させずにk色で塗る方法の数は、返した多項式のkでの評価値。
// nは0..20。多重辺を許す。自己辺があれば全係数0、頂点がなければ{1}。
// m辺に対してO((n+1)^2*2^n+m)時間・O((n+1)*2^n)領域。
// 同梱の subset_convolution / set_series_exp / set_series_log: 法998244353の集合冪級数演算。
// 長さ2^nの列に対し、時間O((n+1)^2*2^n)・領域O((n+1)*2^n)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
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

#ifndef SIRKIT_CHROMATIC_POLYNOMIAL_DEFINED
#define SIRKIT_CHROMATIC_POLYNOMIAL_DEFINED

inline std::vector<std::uint32_t> chromatic_polynomial(
    int n, std::span<const std::pair<int, int>> edges)
{
    constexpr int kMaxVertices = 20;
    constexpr auto kMod = detail::kSetSeriesMod;
    if (n < 0 || n > kMaxVertices)
    {
        throw std::length_error("chromatic_polynomial: vertex count must be in [0,20]");
    }
    using Mask = std::uint32_t;
    std::array<Mask, kMaxVertices> adjacent{};
    bool has_loop = false;
    for (const auto &[u, v] : edges)
    {
        has_loop |= u == v;
        adjacent[u] |= Mask{1} << v;
        adjacent[v] |= Mask{1} << u;
    }
    if (has_loop)
    {
        return std::vector<std::uint32_t>(n + 1);
    }
    if (n == 0)
    {
        return {1};
    }

    const Mask count = Mask{1} << n;
    std::vector<unsigned char> independent(count);
    independent[0] = 1;
    for (Mask mask = 1; mask < count; ++mask)
    {
        const Mask rest = mask & (mask - 1);
        independent[mask] = independent[rest] & ((adjacent[std::countr_zero(mask)] & rest) == 0);
    }

    std::vector<std::uint32_t> partitions(n + 1);
    // 最小頂点を含む独立集合を1色塗る。最初の遷移は残りが全頂点の1状態だけ。
    std::vector<std::uint32_t> remaining(count / 2);
    const auto high_bits = static_cast<Mask>(remaining.size()) - 1;
    for (Mask mask = 0; mask < remaining.size(); ++mask)
    {
        remaining[mask] = independent[2 * (high_bits ^ mask) + 1];
    }
    for (int colors = 1; colors < n; ++colors)
    {
        partitions[colors] = remaining[0];
        // 残りの最小頂点eで分け、eを次の独立集合に入れる。
        // 補集合を取った残り配列との畳み込みで、遷移先Tを含む状態から寄与を集める。
        std::vector<std::uint32_t> next(remaining.size() / 2);
        std::vector<std::uint32_t> eligible(next.size()), reversed(next.size());
        for (int e = colors; e < n; ++e)
        {
            const int shift = e - colors;
            const auto length = next.size() >> shift;
            const auto full = length - 1;
            for (std::size_t mask = 0; mask < length; ++mask)
            {
                eligible[mask] = independent[(2 * mask + 1) << e];
                reversed[full ^ mask] = remaining[(2 * mask + 1) << shift];
            }
            const auto product = subset_convolution(std::span(eligible).first(length),
                                                    std::span(reversed).first(length));
            for (std::size_t mask = 0; mask < length; ++mask)
            {
                auto &value = next[mask << shift];
                const auto sum = value + product[full ^ mask];
                value = sum >= kMod ? sum - kMod : sum;
            }
        }
        remaining = std::move(next);
    }
    partitions[n] = remaining[0];

    // k個の色クラスに色を割り当てる係数はx(x-1)...(x-k+1)。入れ子にして展開する。
    std::vector<std::uint32_t> coefficients(n + 1);
    coefficients[0] = partitions[n];
    const auto subtract_scaled = [](std::uint32_t a, std::uint32_t b, int scale) {
        return static_cast<std::uint32_t>((a + kMod - std::uint64_t{b} * scale % kMod) % kMod);
    };
    for (int k = n - 1; k >= 0; --k)
    {
        for (int degree = n - k; degree > 0; --degree)
        {
            coefficients[degree] = subtract_scaled(coefficients[degree - 1], coefficients[degree], k);
        }
        coefficients[0] = subtract_scaled(partitions[k], coefficients[0], k);
    }
    return coefficients;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
