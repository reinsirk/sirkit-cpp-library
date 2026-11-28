// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Algorithm/LIS_Statistics.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lis_statistics.md
//
// API一覧
// count_lis<Strict=true>(values,modulus=1000000007): 最長増加部分列の長さlengthと個数count（modulusでの余り）。
// modulusは正。値が同じでも添字の選び方が異なれば別。空列では長さ0の部分列が一つ。
// lis_membership<Strict=true>(values): 各添字がLISに一度も現れない・一部に現れる・全てに現れるかを返す。
// LISMembership::kNone、kSome、kAllのいずれか。Strict=falseなら広義単調増加。
// 比較・コピーはO(1)とし、各O((n+1)log(n+2))時間・O(n+1)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LIS_STATISTICS_DEFINED
#define SIRKIT_LIS_STATISTICS_DEFINED

struct LISCount
{
    std::size_t length;
    std::uint64_t count;
};

enum class LISMembership
{
    kNone,
    kSome,
    kAll,
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
// 同じ長さの個数を足すときは、余りの和がuint64_tを越えないよう先に法を減じる。
inline LISCount join_lis_counts(LISCount first, LISCount second, std::uint64_t modulus) noexcept
{
    if (first.length != second.length)
    {
        return first.length > second.length ? first : second;
    }
    const auto gap = modulus - second.count;
    first.count = first.count >= gap ? first.count - gap : first.count + second.count;
    return first;
}

template<bool Strict, class Iterator, class Output, class Compare>
std::size_t lis_scan_lengths(Iterator first, Iterator last, Output output, Compare compare)
{
    std::vector<typename std::iterator_traits<Iterator>::value_type> tails;
    for (; first != last; ++first, ++output)
    {
        const auto position = Strict ? std::lower_bound(tails.begin(), tails.end(), *first, compare)
                                     : std::upper_bound(tails.begin(), tails.end(), *first, compare);
        *output = static_cast<std::size_t>(position - tails.begin()) + 1;
        if (position == tails.end())
        {
            tails.push_back(*first);
        }
        else
        {
            *position = *first;
        }
    }
    return tails.size();
}
} // namespace detail

template<bool Strict = true, class T>
LISCount count_lis(const std::vector<T> &values, std::uint64_t modulus = 1'000'000'007)
{
    if (modulus == 0)
    {
        throw std::invalid_argument("count_lis: modulus must be positive");
    }
    const auto empty_count = std::uint64_t{1} % modulus;
    if (values.empty())
    {
        return {0, empty_count};
    }

    auto sorted = values;
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
    std::vector<LISCount> tree(sorted.size() + 1);
    const auto query = [&](std::size_t prefix) {
        LISCount result{};
        while (prefix != 0)
        {
            result = detail::join_lis_counts(result, tree[prefix], modulus);
            prefix &= prefix - 1;
        }
        return result;
    };
    for (const auto &value : values)
    {
        const auto it = std::lower_bound(sorted.begin(), sorted.end(), value);
        const auto index = static_cast<std::size_t>(it - sorted.begin());
        const auto previous = query(index + !Strict);
        const LISCount candidate{previous.length + 1, previous.length == 0 ? empty_count : previous.count};
        for (auto i = index + 1; i < tree.size(); i += i & (std::size_t{0} - i))
        {
            tree[i] = detail::join_lis_counts(tree[i], candidate, modulus);
        }
    }
    return query(sorted.size());
}

template<bool Strict = true, class T>
std::vector<LISMembership> lis_membership(const std::vector<T> &values)
{
    const auto size = values.size();
    std::vector<std::size_t> left(size), right(size);
    const auto longest = detail::lis_scan_lengths<Strict>(values.begin(), values.end(), left.begin(), std::less<>{});
    detail::lis_scan_lengths<Strict>(values.rbegin(), values.rend(), right.rbegin(),
                                     [](const auto &a, const auto &b) { return b < a; });
    std::vector<std::size_t> candidates(longest + 1);
    std::vector<LISMembership> result(size, LISMembership::kNone);
    for (std::size_t i = 0; i < size; ++i)
    {
        if (left[i] + right[i] - 1 == longest)
        {
            ++candidates[left[i]];
            result[i] = LISMembership::kSome;
        }
    }
    // 各LISは長さの段階ごとに候補を一つ選ぶ。候補が一つなら全てのLISに必要。
    for (std::size_t i = 0; i < size; ++i)
    {
        if (result[i] == LISMembership::kSome && candidates[left[i]] == 1)
        {
            result[i] = LISMembership::kAll;
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
