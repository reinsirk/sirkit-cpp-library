// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_formal_power_series.md
//
// API一覧
// SparseFpsTerm{degree, coefficient}: 形式的冪級数の1項。係数は法998244353で正規化する。
// 次数の順は任意。同次数の係数は足し、次数がn以上の項は無視する。結果は次数0から並べた長さnの係数列。
// sparse_fps_inverse(n, terms): 定数項が非零の級数の逆元。
// sparse_fps_exp(n, terms): 定数項が0の級数の形式指数。
// sparse_fps_log(n, terms): 定数項が1の級数の形式対数。
// sparse_fps_pow(n, terms, exponent): uint64_tの非負整数乗。0乗は1。
// sparse_fps_sqrt(n, terms): 平方根の1つ。存在しなければnullopt。
// 逆元以外はn<=998244353。
// n=0なら空。不正な定数項は通常版でinvalid_argument、積分可能な長さを超えるとlength_error。
// 入力項数Kに対しO(K log(K+1)+n(K+1))時間・O(n+K)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SPARSE_FORMAL_POWER_SERIES_DEFINED
#define SIRKIT_SPARSE_FORMAL_POWER_SERIES_DEFINED

struct SparseFpsTerm
{
    std::size_t degree;
    std::uint32_t coefficient;
};

namespace detail
{
inline constexpr std::uint32_t kSparseFpsMod = 998244353;

inline std::uint32_t SparseFpsAdd(std::uint32_t a, std::uint32_t b)
{
    const auto sum = a + b;
    return sum >= kSparseFpsMod ? sum - kSparseFpsMod : sum;
}

inline std::uint32_t SparseFpsSubtract(std::uint32_t a, std::uint32_t b)
{
    return a >= b ? a - b : kSparseFpsMod - (b - a);
}

inline std::uint32_t SparseFpsMultiply(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kSparseFpsMod);
}

inline std::uint32_t SparseFpsPower(std::uint32_t factor, std::uint64_t exponent)
{
    std::uint32_t result = 1;
    for (; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1) != 0)
        {
            result = SparseFpsMultiply(result, factor);
        }
        factor = SparseFpsMultiply(factor, factor);
    }
    return result;
}

inline std::vector<SparseFpsTerm> NormalizeSparseFps(std::size_t n, std::span<const SparseFpsTerm> input)
{
    std::vector<SparseFpsTerm> terms;
    terms.reserve(input.size());
    for (const auto &[degree, coefficient] : input)
    {
        if (degree < n)
        {
            terms.push_back({degree, coefficient % kSparseFpsMod});
        }
    }
    std::ranges::sort(terms, {}, &SparseFpsTerm::degree);
    std::size_t unique = 0;
    for (const auto [degree, coefficient] : terms)
    {
        if (unique > 0 && terms[unique - 1].degree == degree)
        {
            auto &existing = terms[unique - 1].coefficient;
            existing = SparseFpsAdd(existing, coefficient);
        }
        else
        {
            terms[unique++] = {degree, coefficient};
        }
    }
    terms.resize(unique);
    std::erase_if(terms, [](const auto &term)
                  {
                      return term.coefficient == 0;
                  });
    return terms;
}

inline std::vector<std::uint32_t> SparseFpsIntegerInverses(std::size_t n)
{
    std::vector<std::uint32_t> inverse(n);
    inverse[1] = 1;
    for (std::uint32_t i = 2; i < n; ++i)
    {
        inverse[i] = kSparseFpsMod - SparseFpsMultiply(kSparseFpsMod / i, inverse[kSparseFpsMod % i]);
    }
    return inverse;
}

inline std::uint32_t SparseFpsDot(std::size_t i, std::span<const SparseFpsTerm> terms,
                                  std::span<const std::uint32_t> values)
{
    // 16(p−1)^2+(p−1) < 2^64。積をまとめてから剰余を取る。
    constexpr unsigned kProductsPerReduction = 16;
    unsigned remaining = kProductsPerReduction;
    std::uint64_t sum = 0;
    for (const auto &[degree, coefficient] : terms)
    {
        if (degree > i)
        {
            break;
        }
        sum += std::uint64_t{coefficient} * values[i - degree];
        if (--remaining == 0)
        {
            sum %= kSparseFpsMod;
            remaining = kProductsPerReduction;
        }
    }
    return static_cast<std::uint32_t>(sum % kSparseFpsMod);
}

inline void CheckSparseFpsLength(std::size_t n)
{
    if (n > kSparseFpsMod)
    {
        throw std::length_error("sparse FPS: degree exceeds characteristic");
    }
}
// g = (f/a_0)^exponent に、指定した非零の定数項を掛けた級数を求める。
// f*g'=exponent*f'*g より、g_i = ((exponent+1)/i) Σ_j(j*f_j*g_{i-j}) - Σ_j(f_j*g_{i-j})。
inline void SparseFpsUnitPower(std::span<std::uint32_t> result, std::span<SparseFpsTerm> tail,
                               SparseFpsTerm leading, std::uint32_t exponent, std::uint32_t constant)
{
    result[0] = constant;
    if (result.size() == 1 || tail.empty())
    {
        return;
    }
    const auto inverse_leading = SparseFpsPower(leading.coefficient, kSparseFpsMod - 2);
    const auto factor = SparseFpsAdd(exponent, 1);
    std::vector<SparseFpsTerm> derivative;
    derivative.reserve(tail.size());
    for (auto &[degree, coefficient] : tail)
    {
        degree -= leading.degree;
        coefficient = SparseFpsMultiply(coefficient, inverse_leading);
        const auto weighted = SparseFpsMultiply(static_cast<std::uint32_t>(degree), coefficient);
        derivative.push_back({degree, SparseFpsMultiply(weighted, factor)});
    }
    const auto inverse = SparseFpsIntegerInverses(result.size());
    for (std::size_t i = 1; i < result.size(); ++i)
    {
        const auto derivative_sum = SparseFpsMultiply(SparseFpsDot(i, derivative, result), inverse[i]);
        result[i] = SparseFpsSubtract(derivative_sum, SparseFpsDot(i, tail, result));
    }
}

// 998244353 = 119*2^23+1。非平方剰余3からTonelli–Shanks法で平方根を求める。
inline std::optional<std::uint32_t> SparseFpsScalarSquareRoot(std::uint32_t value)
{
    if (SparseFpsPower(value, (kSparseFpsMod - 1) / 2) != 1)
    {
        return std::nullopt;
    }
    constexpr unsigned kOdd = 119;
    unsigned depth = 23;
    auto root = SparseFpsPower(value, (kOdd + 1) / 2);
    auto residue = SparseFpsPower(value, kOdd);
    auto step = SparseFpsPower(3, kOdd);
    while (residue != 1)
    {
        unsigned order = 0;
        for (auto power = residue; power != 1; ++order)
        {
            power = SparseFpsMultiply(power, power);
        }
        auto adjust = step;
        for (unsigned i = order + 1; i < depth; ++i)
        {
            adjust = SparseFpsMultiply(adjust, adjust);
        }
        root = SparseFpsMultiply(root, adjust);
        step = SparseFpsMultiply(adjust, adjust);
        residue = SparseFpsMultiply(residue, step);
        depth = order;
    }
    return std::min(root, kSparseFpsMod - root);
}
} // namespace detail

inline std::vector<std::uint32_t> sparse_fps_inverse(std::size_t n, std::span<const SparseFpsTerm> input)
{
    if (n == 0)
    {
        return {};
    }
    auto terms = detail::NormalizeSparseFps(n, input);
#ifndef SKIP_RUNTIME_CHECKS
    if (terms.empty() || terms.front().degree != 0)
    {
        throw std::invalid_argument("sparse_fps_inverse: zero constant term");
    }
#endif
    const auto inverse = detail::SparseFpsPower(terms.front().coefficient, detail::kSparseFpsMod - 2);
    std::vector<std::uint32_t> result(n);
    result[0] = inverse;
    if (terms.size() > 1)
    {
        auto tail = std::span(terms).subspan(1);
        for (auto &[degree, coefficient] : tail)
        {
            coefficient = detail::kSparseFpsMod - detail::SparseFpsMultiply(coefficient, inverse);
        }
        // f*g=1 の次数iの係数から、g[i]をそれより低い係数だけで求める。
        for (std::size_t i = 1; i < n; ++i)
        {
            result[i] = detail::SparseFpsDot(i, tail, result);
        }
    }
    return result;
}

inline std::vector<std::uint32_t> sparse_fps_exp(std::size_t n, std::span<const SparseFpsTerm> input)
{
    if (n == 0)
    {
        return {};
    }
    detail::CheckSparseFpsLength(n);
    auto terms = detail::NormalizeSparseFps(n, input);
#ifndef SKIP_RUNTIME_CHECKS
    if (!terms.empty() && terms.front().degree == 0)
    {
        throw std::invalid_argument("sparse_fps_exp: nonzero constant term");
    }
#endif
    std::vector<std::uint32_t> result(n);
    result[0] = 1;
    if (!terms.empty())
    {
        for (auto &[degree, coefficient] : terms)
        {
            coefficient = detail::SparseFpsMultiply(static_cast<std::uint32_t>(degree), coefficient);
        }
        // g'=f'*g の次数i-1から、i*g[i]を求める。
        const auto inverse = detail::SparseFpsIntegerInverses(n);
        for (std::size_t i = 1; i < n; ++i)
        {
            result[i] = detail::SparseFpsMultiply(detail::SparseFpsDot(i, terms, result), inverse[i]);
        }
    }
    return result;
}

inline std::vector<std::uint32_t> sparse_fps_log(std::size_t n, std::span<const SparseFpsTerm> input)
{
    if (n == 0)
    {
        return {};
    }
    detail::CheckSparseFpsLength(n);
    auto terms = detail::NormalizeSparseFps(n, input);
#ifndef SKIP_RUNTIME_CHECKS
    if (terms.empty() || terms.front().degree != 0 || terms.front().coefficient != 1)
    {
        throw std::invalid_argument("sparse_fps_log: constant term must be one");
    }
#endif
    std::vector<std::uint32_t> result(n);
    if (terms.size() > 1)
    {
        auto tail = std::span(terms).subspan(1);
        for (auto &[degree, coefficient] : tail)
        {
            result[degree] = detail::SparseFpsMultiply(static_cast<std::uint32_t>(degree), coefficient);
            coefficient = detail::kSparseFpsMod - coefficient;
        }
        // f*h=x*f' の解hを先に計算し、h_i/iで対数の係数を得る。
        for (std::size_t i = 1; i < n; ++i)
        {
            result[i] = detail::SparseFpsAdd(result[i], detail::SparseFpsDot(i, tail, result));
        }
        const auto inverse = detail::SparseFpsIntegerInverses(n);
        for (std::size_t i = 1; i < n; ++i)
        {
            result[i] = detail::SparseFpsMultiply(result[i], inverse[i]);
        }
    }
    return result;
}

inline std::vector<std::uint32_t> sparse_fps_pow(std::size_t n, std::span<const SparseFpsTerm> input,
                                                 std::uint64_t exponent)
{
    if (n == 0)
    {
        return {};
    }
    detail::CheckSparseFpsLength(n);
    std::vector<std::uint32_t> result(n);
    if (exponent == 0)
    {
        result[0] = 1;
        return result;
    }
    auto terms = detail::NormalizeSparseFps(n, input);
    if (terms.empty() || terms.front().degree > (n - 1) / exponent)
    {
        return result;
    }
    const auto leading = terms.front();
    const auto shift = static_cast<std::size_t>(leading.degree * exponent);
    const auto constant = detail::SparseFpsPower(leading.coefficient, exponent);
    const auto field_exponent = static_cast<std::uint32_t>(exponent % detail::kSparseFpsMod);
    detail::SparseFpsUnitPower(std::span(result).subspan(shift), std::span(terms).subspan(1), leading,
                               field_exponent, constant);
    return result;
}

inline std::optional<std::vector<std::uint32_t>> sparse_fps_sqrt(std::size_t n, std::span<const SparseFpsTerm> input)
{
    if (n == 0)
    {
        return std::vector<std::uint32_t>{};
    }
    detail::CheckSparseFpsLength(n);
    auto terms = detail::NormalizeSparseFps(n, input);
    std::vector<std::uint32_t> result(n);
    if (terms.empty())
    {
        return result;
    }
    const auto leading = terms.front();
    if (leading.degree % 2 != 0)
    {
        return std::nullopt;
    }
    const auto constant = detail::SparseFpsScalarSquareRoot(leading.coefficient);
    if (!constant)
    {
        return std::nullopt;
    }
    constexpr auto kHalf = (detail::kSparseFpsMod + 1) / 2;
    detail::SparseFpsUnitPower(std::span(result).subspan(leading.degree / 2, n - leading.degree),
                               std::span(terms).subspan(1), leading, kHalf, *constant);
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
