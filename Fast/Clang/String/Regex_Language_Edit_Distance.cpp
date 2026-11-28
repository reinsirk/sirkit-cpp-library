// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Regex_Language_Edit_Distance.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/regex_language_edit_distance.md
//
// API一覧
// regex_language_edit_distance(a,b): 各パターンに一致する文字列を1個ずつ選んだときの最小編集距離。
// パターンは英小文字と、その文字の直後の ?（0回か1回）、*（0回以上）だけ。空のパターンも可。
// 挿入・削除・置換の各コストは1。不正なパターンはチェック有効時にinvalid_argument。
// 生のパターン長n,mでO(n*m+n+m+1)時間、O(n+m+1)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_REGEX_LANGUAGE_EDIT_DISTANCE_DEFINED
#define SIRKIT_REGEX_LANGUAGE_EDIT_DISTANCE_DEFINED

namespace detail
{
struct RegexDistanceAtom
{
    char letter;
    char repetition;

    [[nodiscard]] constexpr bool repeated() const noexcept
    {
        return repetition == '*';
    }

    [[nodiscard]] constexpr std::size_t skip_cost() const noexcept
    {
        return repetition == '\0';
    }
};

inline std::vector<RegexDistanceAtom> ParseRegexDistancePattern(std::string_view pattern)
{
    std::vector<RegexDistanceAtom> atoms;
    atoms.reserve(pattern.size());
    for (std::size_t i = 0; i < pattern.size(); ++i)
    {
        const char letter = pattern[i];
        char repetition = '\0';
        if (i + 1 < pattern.size() && (pattern[i + 1] == '?' || pattern[i + 1] == '*'))
        {
            repetition = pattern[++i];
        }
        atoms.push_back({letter, repetition});
    }
    return atoms;
}
}

[[nodiscard]] inline std::size_t regex_language_edit_distance(std::string_view a, std::string_view b)
{
    auto rows = detail::ParseRegexDistancePattern(a), columns = detail::ParseRegexDistancePattern(b);
    if (rows.size() < columns.size())
    {
        rows.swap(columns);
    }
    const auto m = columns.size();
    std::vector<std::size_t> next(m + 1), current(m + 1);
    for (std::size_t j = m; j > 0; --j)
    {
        next[j - 1] = next[j] + columns[j - 1].skip_cost();
    }
    for (std::size_t i = rows.size(); i > 0; --i)
    {
        const auto &left = rows[i - 1];
        current[m] = next[m] + left.skip_cost();
        for (std::size_t j = m; j > 0; --j)
        {
            const auto &right = columns[j - 1];
            auto best = std::min(next[j - 1] + left.skip_cost(), current[j] + right.skip_cost());
            const auto mismatch = static_cast<std::size_t>(left.letter != right.letter);
            // 両側の * が同時に文字を出す遷移は、状態を進めないので省ける。
            if (left.repeated())
            {
                if (!right.repeated())
                {
                    best = std::min(best, current[j] + mismatch);
                }
            }
            else
            {
                const auto remainder = right.repeated() ? next[j - 1] : next[j];
                best = std::min(best, remainder + mismatch);
            }
            current[j - 1] = best;
        }
        next.swap(current);
    }
    return next.front();
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
