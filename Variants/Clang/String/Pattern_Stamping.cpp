// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: String/Pattern_Stamping.cpp
// 参考（上書き操作の定義）: https://atcoder.jp/contests/abc329/tasks/abc329_e
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pattern_stamping.md
//
// API一覧
// pattern_stamping_order(target,stamp): 全位置が未記入の列を、stampによる連続区間の上書きでtargetにする。
// 可能なときは0始まりの開始位置を操作順に返す。作れなければnullopt。空targetには空の操作列を返す。
// 文字の一致はバイト単位で、予約文字はない。操作数は|target|以下。
// n=|target|、m=|stamp|。O(1+n*m)時間、O(n+1)領域。短いstamp向け。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PATTERN_STAMPING_DEFINED
#define SIRKIT_PATTERN_STAMPING_DEFINED

[[nodiscard]] inline std::optional<std::vector<std::size_t>> pattern_stamping_order(
    std::string_view target, std::string_view stamp)
{
    const std::size_t n = target.size(), m = stamp.size();
    if (n == 0)
    {
        return std::vector<std::size_t>{};
    }
    if (m == 0 || m > n)
    {
        return std::nullopt;
    }

    const std::size_t windows = n - m + 1;
    std::vector<std::size_t> mismatches(windows), ready;
    for (std::size_t left = 0; left < windows; ++left)
    {
        for (std::size_t i = 0; i < m; ++i)
        {
            mismatches[left] += target[left + i] != stamp[i];
        }
        if (mismatches[left] == 0)
        {
            ready.push_back(left);
        }
    }

    std::vector<unsigned char> erased(n, 0);
    std::vector<std::size_t> order;
    std::size_t remaining = n;
    for (std::size_t head = 0; head < ready.size() && remaining != 0; ++head)
    {
        const std::size_t left = ready[head];
        const std::size_t before = remaining;
        for (std::size_t point = left; point < left + m; ++point)
        {
            if (erased[point])
            {
                continue;
            }
            erased[point] = 1;
            --remaining;
            // この位置を消すと、ここを含む区間が持つ不一致だけが減る。
            const std::size_t first = point - std::min(point, m - 1);
            const std::size_t last = std::min(point, windows - 1);
            for (std::size_t other = first; other <= last; ++other)
            {
                if (target[point] != stamp[point - other] && --mismatches[other] == 0)
                {
                    ready.push_back(other);
                }
            }
        }
        if (remaining < before)
        {
            order.push_back(left);
        }
    }

    if (remaining != 0)
    {
        return std::nullopt;
    }
    std::ranges::reverse(order);
    return order;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
