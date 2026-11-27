// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Longest_Common_Subsequence.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/longest_common_subsequence.md
//
// API一覧
// longest_common_subsequence(a,b): 最長共通部分列を1つ復元する。共通文字がなければ空。
// 複数解の選択は任意。O(n*m+1)時間・O(min(n,m)+log(max(n,m)+1))領域。入力は変更しない。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LONGEST_COMMON_SUBSEQUENCE_DEFINED
#define SIRKIT_LONGEST_COMMON_SUBSEQUENCE_DEFINED

inline std::string longest_common_subsequence(std::string_view a, std::string_view b)
{
    if (a.size() < b.size())
    {
        std::swap(a, b);
    }
    std::string result;
    result.reserve(b.size());

    const auto lengths = []<bool Reverse>(std::string_view rows, std::string_view columns)
    {
        std::vector<std::size_t> table(columns.size() + 1);
        for (std::size_t i = 0; i < rows.size(); ++i)
        {
            const char letter = rows[Reverse ? rows.size() - 1 - i : i];
            std::size_t previous = 0;
            for (std::size_t j = 0; j < columns.size(); ++j)
            {
                const auto diagonal = previous;
                previous = table[j + 1];
                const char other = columns[Reverse ? columns.size() - 1 - j : j];
                table[j + 1] = letter == other ? diagonal + 1 : std::max(table[j], previous);
            }
        }
        return table;
    };

    const auto reconstruct = [&](auto &&self, std::string_view rows, std::string_view columns) -> void
    {
        if (columns.empty())
        {
            return;
        }
        if (rows.size() == 1)
        {
            if (columns.find(rows.front()) != std::string_view::npos)
            {
                result.push_back(rows.front());
            }
            return;
        }
        const auto middle = rows.size() / 2;
        std::size_t split = 0;
        // 前半と後半のDPを合わせ、共通部分列を分ける境界を決める。
        {
            const auto before = lengths.template operator()<false>(rows.substr(0, middle), columns);
            const auto after = lengths.template operator()<true>(rows.substr(middle), columns);
            std::size_t best = 0;
            for (std::size_t j = 0; j <= columns.size(); ++j)
            {
                const auto total = before[j] + after[columns.size() - j];
                if (total > best)
                {
                    best = total;
                    split = j;
                }
            }
        }
        self(self, rows.substr(0, middle), columns.substr(0, split));
        self(self, rows.substr(middle), columns.substr(split));
    };

    reconstruct(reconstruct, a, b);
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
