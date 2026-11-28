// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Dyadic_Interval_Cover.cpp
// 参考（2冪長の区間の定義とVerify）: https://atcoder.jp/contests/abc349/tasks/abc349_d
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dyadic_interval_cover.md
//
// API一覧
// dyadic_interval_cover(left,right): 半開区間[left,right)を、長さ2^kで始点がその倍数の最少個数の区間に分ける。
// 区間をpair{left,right}の昇順列で返す。両端は同じ標準整数型、bool以外。負の座標も可。
// left==rightなら空、left>rightならinvalid_argument。時間・領域ともにO(1+区間数)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DYADIC_INTERVAL_COVER
#define SIRKIT_DYADIC_INTERVAL_COVER

template <std::integral Integer>
    requires(!std::same_as<Integer, bool>)
[[nodiscard]] constexpr std::vector<std::pair<Integer, Integer>> dyadic_interval_cover(Integer left, Integer right)
{
    if (left > right)
    {
        throw std::invalid_argument("dyadic_interval_cover: reversed interval");
    }
    using Unsigned = std::make_unsigned_t<Integer>;
    std::vector<std::pair<Integer, Integer>> result;
    while (left != right)
    {
        // 符号付き全域でも、残りの幅は対応する符号なし型に収まる。
        const auto first = static_cast<Unsigned>(left);
        const auto remaining = static_cast<Unsigned>(static_cast<Unsigned>(right) - first);
        auto width = std::bit_floor(remaining);
        if (first != 0)
        {
            const auto alignment = static_cast<Unsigned>(Unsigned{1} << std::countr_zero(first));
            width = std::min(width, alignment);
        }
        const auto next = static_cast<Integer>(static_cast<Unsigned>(first + width));
        result.emplace_back(left, next);
        left = next;
    }
    return result;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
