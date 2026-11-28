// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Floor_Path_Product.cpp
// 参考（floor sumのモノイド積）: https://atcoder.jp/contests/arc183/editorial/10768
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/floor_path_product.md
//
// API一覧
// floor_path_product(n,m,a,b,horizontal,vertical,identity,combine):
// y_i=floor((a*i+b)/m)とし、vertical^y_0 と各 horizontal vertical^(y_(i+1)-y_i) (0<=i<n) の積。
// n,mはuint32_t、m>0。a,bは非負uint64_t。combineの結合則とidentityが必要。可換性は不要。
// O(log(n+1)+log(m+1)+log(a+1)+log(b+1))回のcombine、O(log(m+1))領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstdint>
#include <stdexcept>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FLOOR_PATH_PRODUCT_DEFINED
#define SIRKIT_FLOOR_PATH_PRODUCT_DEFINED
namespace detail
{
template <class T, class Combine>
T FloorWordPower(T base, std::uint64_t exponent, const T &identity, const Combine &combine)
{
    T result = identity;
    for (; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1U) != 0)
        {
            result = combine(result, base);
        }
        if (exponent > 1)
        {
            base = combine(base, base);
        }
    }
    return result;
}

template <class T, class Combine>
T FloorWordProduct(std::uint64_t n, std::uint64_t m, std::uint64_t a, std::uint64_t b,
                   T horizontal, const T &vertical, const T &identity, const Combine &combine)
{
    const auto initial = FloorWordPower(vertical, b / m, identity, combine);
    if (n == 0)
    {
        return initial;
    }
    horizontal = combine(horizontal, FloorWordPower(vertical, a / m, identity, combine));
    a %= m;
    b %= m;
    const auto height = (a * n + b) / m;
    if (height == 0)
    {
        return combine(initial, FloorWordPower(horizontal, n, identity, combine));
    }

    // j番目の上移動の直前に横移動をceil((m*j-b)/a)回終えている。
    const auto transposed = FloorWordProduct(height - 1, a, m, m + a - 1 - b,
                                             vertical, horizontal, identity, combine);
    const auto last_column = (m * height + a - 1 - b) / a;
    return combine(initial, combine(transposed, combine(vertical, FloorWordPower(horizontal, n - last_column, identity, combine))));
}
}

template <class T, class Combine>
T floor_path_product(std::uint32_t n, std::uint32_t m, std::uint64_t a, std::uint64_t b,
                     const T &horizontal, const T &vertical, const T &identity, const Combine &combine)
{
    if (m == 0)
    {
        throw std::invalid_argument("floor_path_product: positive modulus required");
    }
    return detail::FloorWordProduct(n, m, a, b, horizontal, vertical, identity, combine);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
