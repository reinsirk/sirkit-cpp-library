// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Floor_Sum_Moments.cpp
// 参考（一般化floor sumの応用）: https://atcoder.jp/contests/abc402/editorial/12688
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/floor_sum_moments.md
//
// API一覧
// [FloorSumMoments]
// sum, index_weighted, pairs: q_i=floor((a*i+b)/m)のΣq_i、Σi*q_i、Σq_i*(q_i-1)/2。
// sum_of_squares(): Σq_i^2をO(1)で返す。全ての値は法2^64の余り。
// floor_sum_moments(n,m,a,b): 0<=i<nの3つの総和。n,mはuint32_t、m>0。
// a,bは負も可。n=0なら全て0。O(log(m+1))時間・領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstdint>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FLOOR_SUM_MOMENTS_DEFINED
#define SIRKIT_FLOOR_SUM_MOMENTS_DEFINED

struct FloorSumMoments
{
    std::uint64_t sum{};
    std::uint64_t index_weighted{};
    std::uint64_t pairs{};

    [[nodiscard]] std::uint64_t sum_of_squares() const noexcept
    {
        return 2 * pairs + sum;
    }
};

namespace detail
{
inline std::uint64_t FloorMomentChooseTwo(std::int64_t value) noexcept
{
    const auto residue = static_cast<std::uint64_t>(value);
    if (value % 2 == 0)
    {
        return static_cast<std::uint64_t>(value / 2) * (residue - 1);
    }
    return residue * static_cast<std::uint64_t>((value - 1) / 2);
}

inline std::uint64_t FloorMomentChooseThree(std::uint64_t n) noexcept
{
    if (n < 3)
    {
        return 0;
    }
    auto a = n;
    auto b = n - 1;
    auto c = n - 2;
    if (a % 2 == 0)
    {
        a /= 2;
    }
    else
    {
        b /= 2;
    }
    if (a % 3 == 0)
    {
        a /= 3;
    }
    else if (b % 3 == 0)
    {
        b /= 3;
    }
    else
    {
        c /= 3;
    }
    return a * b * c;
}

inline std::pair<std::int64_t, std::uint64_t> FloorMomentDivide(std::int64_t value, std::uint32_t modulus) noexcept
{
    auto div = value / modulus;
    auto rem = value % modulus;
    if (rem < 0)
    {
        --div;
        rem += modulus;
    }
    return {div, static_cast<std::uint64_t>(rem)};
}

inline FloorSumMoments FloorMoments(std::uint64_t n, std::uint32_t modulus, std::int64_t a, std::int64_t b) noexcept
{
    const auto [quotient_a, slope] = FloorMomentDivide(a, modulus);
    const auto [quotient_b, intercept] = FloorMomentDivide(b, modulus);
    const auto index_sum = n * (n - 1) / 2;
    const auto index_pairs = FloorMomentChooseThree(n);
    const auto index_squares = 2 * index_pairs + index_sum;

    const auto height = (slope * (n - 1) + intercept) / modulus;
    FloorSumMoments reduced;
    if (height != 0)
    {
        // j=0..height-1に対し、q_i>=j+1となる最小のiを求める。
        const auto threshold = modulus + slope - intercept - 1;
        const auto transposed = FloorMoments(height, static_cast<std::uint32_t>(slope), modulus,
                                             static_cast<std::int64_t>(threshold));
        reduced.sum = n * height - transposed.sum;
        reduced.index_weighted = index_sum * height - transposed.pairs;
        reduced.pairs = n * (height * (height - 1) / 2) - transposed.index_weighted;
    }

    // 捨てた整数部分A*i+Bを戻す。C(u+v,2)=C(u,2)+C(v,2)+uv。
    const auto linear_a = static_cast<std::uint64_t>(quotient_a);
    const auto linear_b = static_cast<std::uint64_t>(quotient_b);
    const auto linear_pairs = linear_a * linear_a * index_pairs +
                              (FloorMomentChooseTwo(quotient_a) + linear_a * linear_b) * index_sum +
                              FloorMomentChooseTwo(quotient_b) * n;
    return {reduced.sum + linear_a * index_sum + linear_b * n,
            reduced.index_weighted + linear_a * index_squares + linear_b * index_sum,
            reduced.pairs + linear_a * reduced.index_weighted + linear_b * reduced.sum + linear_pairs};
}
}

inline FloorSumMoments floor_sum_moments(std::uint32_t n, std::uint32_t m, std::int64_t a, std::int64_t b)
{
    if (m == 0)
    {
        throw std::invalid_argument("floor_sum_moments: positive modulus required");
    }
    if (n == 0)
    {
        return {};
    }
    return detail::FloorMoments(n, m, a, b);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
