// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Floor_Sum.cpp
// 参考: https://nyaannyaan.github.io/library/math/floor-sum.hpp
// API仕様: https://atcoder.github.io/ac-library/production/document_ja/math.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/floor_sum.md
//
// API一覧
// floor_sum(n,m,a,b): i=0..n-1 の floor((a*i+b)/m) の総和。a,bは負でもよい。
// 0<=n<2^32、1<=m<2^32。ACLと同じく結果は2^64を法とする符号付き64ビット値。
// min_of_mod_of_linear(n,m,a,b): i=0..n-1 の (a*i+b) mod m の最小値。1<=n<2^32。
// 両方ともO(log(m+1))時間・O(1)領域。検査有効時、n,mが範囲外ならstd::invalid_argument。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FLOOR_SUM_DEFINED
#define SIRKIT_FLOOR_SUM_DEFINED

inline std::int64_t floor_sum(std::int64_t n, std::int64_t m, std::int64_t a, std::int64_t b)
{
    if (n == 0)
    {
        return 0;
    }
    const auto count = static_cast<std::uint64_t>(n);
    const auto triangular = count * (count - 1) / 2;
    std::uint64_t sum = 0;
    if (a < 0)
    {
        const auto remainder = a % m;
        const auto quotient = a / m;
        sum += triangular * static_cast<std::uint64_t>(quotient - (remainder != 0));
        a = remainder == 0 ? 0 : remainder + m;
    }
    if (b < 0)
    {
        const auto remainder = b % m;
        const auto quotient = b / m;
        sum += count * static_cast<std::uint64_t>(quotient - (remainder != 0));
        b = remainder == 0 ? 0 : remainder + m;
    }
    auto length = count;
    auto modulus = static_cast<std::uint64_t>(m);
    auto slope = static_cast<std::uint64_t>(a);
    auto offset = static_cast<std::uint64_t>(b);
    while (true)
    {
        if (slope >= modulus)
        {
            sum += (length * (length - 1) / 2) * (slope / modulus);
            slope %= modulus;
        }
        if (offset >= modulus)
        {
            sum += length * (offset / modulus);
            offset %= modulus;
        }
        const auto limit = slope * length + offset;
        if (limit < modulus)
        {
            break;
        }
        length = limit / modulus; // 格子点を横方向から数え直す。
        offset = limit % modulus;
        std::swap(modulus, slope);
    }
    return std::bit_cast<std::int64_t>(sum);
}

inline std::int64_t min_of_mod_of_linear(std::int64_t n, std::int64_t m, std::int64_t a, std::int64_t b)
{
    const auto normalize = [](std::int64_t x, std::int64_t modulus)
    {
        x %= modulus;
        return static_cast<std::uint64_t>(x < 0 ? x + modulus : x);
    };
    auto length = static_cast<std::uint64_t>(n);
    auto modulus = static_cast<std::uint64_t>(m);
    auto slope = normalize(a, m);
    auto offset = normalize(b, m);
    auto minimum = offset;
    while (minimum != 0 && slope != 0)
    {
        if (slope * 2 > modulus) // 逆から走査して次の法を半分以下にする。
        {
            offset = (slope * (length - 1) + offset) % modulus;
            slope = modulus - slope;
        }
        minimum = std::min(minimum, offset);
        const auto wraps = (slope * (length - 1) + offset) / modulus;
        if (wraps == 0)
        {
            break;
        }
        const auto reverse_step = (slope - modulus % slope) % slope; // 次の法は slope。
        length = wraps;
        offset = (offset % slope + reverse_step) % slope; // y回目の折り返しは (offset - modulus*y) mod slope。
        modulus = slope;
        slope = reverse_step;
        minimum = std::min(minimum, offset);
    }
    return static_cast<std::int64_t>(minimum);
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
