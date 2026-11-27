// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: String/Edit_Distance.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/edit_distance.md
//
// API一覧
// edit_distance(a,b): 1文字の挿入・削除・置換を各1回とした最小編集回数。O(n*m+1)時間・O(min(n,m)+1)領域。
// edit_distance_at_most(a,b,k): 距離がk以下ならその値、超えるならnullopt。
// k<max(n,m)のとき O((max(n,m)+1)*min(2*k+1,min(n,m)+1))時間・O(min(k,min(n,m))+1)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <numeric>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EDIT_DISTANCE_DEFINED
#define SIRKIT_EDIT_DISTANCE_DEFINED

inline std::size_t edit_distance(std::string_view a, std::string_view b)
{
    if (a.size() < b.size())
    {
        std::swap(a, b);
    }
    if (b.empty())
    {
        return a.size();
    }
    std::vector<std::size_t> row(b.size() + 1);
    std::iota(row.begin(), row.end(), std::size_t{0});
    for (char c : a)
    {
        auto diagonal = row[0]++;
        for (std::size_t j = 0; j < b.size(); ++j)
        {
            const auto above = row[j + 1];
            row[j + 1] = std::min({above + 1, row[j] + 1, diagonal + (c != b[j])});
            diagonal = above;
        }
    }
    return row.back();
}

inline std::optional<std::size_t> edit_distance_at_most(std::string_view a, std::string_view b, std::size_t limit)
{
    if (a.size() < b.size())
    {
        std::swap(a, b);
    }
    const auto n = a.size(), m = b.size();
    if (n - m > limit)
    {
        return std::nullopt;
    }
    if (limit >= n)
    {
        return edit_distance(a, b);
    }
    const auto unreachable = limit + 1;
    const auto increment = [unreachable](std::size_t value)
    {
        return value < unreachable ? value + 1 : value;
    };
    const auto width = limit >= m / 2 ? m + 1 : 2 * limit + 1;
    std::vector<std::size_t> previous(width), current(width);
    std::iota(previous.begin(), previous.end(), std::size_t{0});
    std::size_t previous_left = 0, previous_right = std::min(m, limit);
    for (std::size_t i = 1; i <= n; ++i)
    {
        const auto left = i > limit ? i - limit : 0;
        const auto right = i >= m || limit >= m - i ? m : i + limit;
        const auto from_previous = [&](std::size_t j)
        {
            return j <= previous_right ? previous[j - previous_left] : unreachable;
        };
        for (std::size_t j = left; j <= right; ++j)
        {
            if (j == 0)
            {
                current[0] = i;
                continue;
            }
            const auto insertion = j == left ? unreachable : increment(current[j - left - 1]);
            const auto deletion = increment(from_previous(j));
            auto replacement = from_previous(j - 1);
            if (a[i - 1] != b[j - 1])
            {
                replacement = increment(replacement);
            }
            current[j - left] = std::min({insertion, deletion, replacement});
        }
        previous.swap(current);
        previous_left = left;
        previous_right = right;
    }
    const auto answer = previous[m - previous_left];
    return answer <= limit ? std::optional{answer} : std::nullopt;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
