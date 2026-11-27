// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Floor_Quotients.cpp
// 参考（商の区間列挙）: https://nyaannyaan.github.io/library/math/enumerate-quotient.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/floor_quotients.md
//
// API一覧
// for_each_floor_quotient(n,visit): 1<=i<=nでfloor(n/i)が一定の各閉区間をi昇順に列挙する。
// visit(first,last,quotient)を区間ごとに呼ぶ。nはuint64_t。O(sqrt(n+1))時間・O(1)追加領域。
// enumerate_quotients(n): 相異なる商を昇順で返す。O(sqrt(n+1))時間・領域。
// n=0ではvisitを呼ばず、商の列は空。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstdint>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FLOOR_QUOTIENTS_DEFINED
#define SIRKIT_FLOOR_QUOTIENTS_DEFINED

template <class Visit>
void for_each_floor_quotient(std::uint64_t n, Visit &&visit)
{
    for (std::uint64_t first = 1; first <= n;)
    {
        const auto quotient = n / first;
        const auto last = first <= quotient ? first : n / quotient;
        visit(first, last, quotient);
        if (last == n)
        {
            return;
        }
        first = last + 1;
    }
}

inline std::vector<std::uint64_t> enumerate_quotients(std::uint64_t n)
{
    std::vector<std::uint64_t> result;
    for_each_floor_quotient(n, [&](std::uint64_t, std::uint64_t, std::uint64_t quotient)
                            {
                                result.push_back(quotient);
                            });
    std::reverse(result.begin(), result.end());
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
