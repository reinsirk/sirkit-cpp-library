// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Gaussian_Integer_GCD.cpp
// 数学的参考: K. Conrad, The Gaussian Integers, §§3–4 (https://kconrad.math.uconn.edu/blurbs/ugradnumthy/Zinotes.pdf)
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/gaussian_integer_gcd.md
//
// API一覧
// GaussianInteger: real, imagでa+biを表す。成分型はint64_t。
// gaussian_gcd(a, b): ガウス整数の最大公約数を返す。入力の各成分は[-2147483647,2147483647]。
// 非零の結果は実部が正で虚部が非負。両方が0なら0。通常版で範囲外はout_of_range。
// 成分の絶対値の最大をMとしてO(log(M+2))時間、O(1)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstdint>
#include <limits>
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GAUSSIAN_INTEGER_GCD_DEFINED
#define SIRKIT_GAUSSIAN_INTEGER_GCD_DEFINED

struct GaussianInteger
{
    std::int64_t real = 0;
    std::int64_t imag = 0;

    friend constexpr bool operator==(const GaussianInteger &left, const GaussianInteger &right) noexcept
    {
        return left.real == right.real && left.imag == right.imag;
    }
};

namespace detail
{
inline std::int64_t GaussianRoundQuotient(std::int64_t numerator, std::int64_t denominator)
{
    const auto quotient = numerator / denominator;
    const auto residue = numerator % denominator;
    // 余りの倍算を避ける。同率では0から遠い整数に丸める。
    const auto threshold = denominator - denominator / 2;
    return quotient + (residue >= threshold) - (residue <= -threshold);
}
} // namespace detail

inline GaussianInteger gaussian_gcd(GaussianInteger a, GaussianInteger b)
{
#ifndef SKIP_RUNTIME_CHECKS
    constexpr auto kBound = std::numeric_limits<std::int32_t>::max();
    const auto valid = [](const GaussianInteger &z)
    {
        return -kBound <= z.real && z.real <= kBound && -kBound <= z.imag && z.imag <= kBound;
    };
    if (!valid(a) || !valid(b))
    {
        throw std::out_of_range("gaussian_gcd: input coordinates outside the supported range");
    }
#endif
    while (b != GaussianInteger{})
    {
        // 商の各成分を最近整数に丸めると、剰余のノルムは除数のノルムの半分以下。
        // 初期ノルムは2M²以下なので、以降のどの成分の絶対値もMを超えない。
        const auto norm = b.real * b.real + b.imag * b.imag;
        const auto real = detail::GaussianRoundQuotient(a.real * b.real + a.imag * b.imag, norm);
        const auto imag = detail::GaussianRoundQuotient(a.imag * b.real - a.real * b.imag, norm);
        const GaussianInteger remainder{a.real - (real * b.real - imag * b.imag),
                                        a.imag - (real * b.imag + imag * b.real)};
        a = b;
        b = remainder;
    }
    if (a != GaussianInteger{})
    {
        // i倍を繰り返し、非零の単元倍のうち第一象限（正の実軸を含む）を選ぶ。
        while (a.real <= 0 || a.imag < 0)
        {
            a = {-a.imag, a.real};
        }
    }
    return a;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
