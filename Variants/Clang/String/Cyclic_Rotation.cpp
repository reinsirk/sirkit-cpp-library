// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: String/Cyclic_Rotation.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/cyclic_rotation.md
//
// API一覧
// min_cyclic_rotation_index(text): 辞書順で最小の巡回シフトの開始位置を返す。
// max_cyclic_rotation_index(text): 辞書順で最大の巡回シフトの開始位置を返す。
// 文字は符号なしバイトとして比較。同順位なら最小の開始位置、空文字なら0。各O(n+1)時間・O(1)追加領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <string_view>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CYCLIC_ROTATION_DEFINED
#define SIRKIT_CYCLIC_ROTATION_DEFINED

namespace detail
{
inline std::size_t cyclic_rotation_offset(std::size_t start, std::size_t offset, std::size_t length) noexcept
{
    const std::size_t tail = length - start;
    return offset < tail ? start + offset : offset - tail;
}

template<bool Maximize>
std::size_t cyclic_rotation_index(std::string_view text) noexcept
{
    const std::size_t length = text.size();
    std::size_t first = 0;
    std::size_t second = 1;
    std::size_t matched = 0;
    while (first < length && second < length && matched < length)
    {
        const auto a = static_cast<unsigned char>(text[cyclic_rotation_offset(first, matched, length)]);
        const auto b = static_cast<unsigned char>(text[cyclic_rotation_offset(second, matched, length)]);
        if (a == b)
        {
            ++matched;
            continue;
        }

        const bool first_loses = Maximize ? a < b : a > b;
        auto &loser = first_loses ? first : second;
        const auto winner = first_loses ? second : first;
        // 一致部分をずらしても最初の大小関係は同じなので、負けた候補の先頭から一致長までを除く。
        loser += std::min(matched + 1, length - loser);
        if (loser == winner)
        {
            ++loser;
        }
        matched = 0;
    }
    return std::min(first, second);
}
} // namespace detail

inline std::size_t min_cyclic_rotation_index(std::string_view text) noexcept
{
    return detail::cyclic_rotation_index<false>(text);
}

inline std::size_t max_cyclic_rotation_index(std::string_view text) noexcept
{
    return detail::cyclic_rotation_index<true>(text);
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
