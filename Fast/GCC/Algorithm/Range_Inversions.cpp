// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Range_Inversions.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_inversions.md
//
// API一覧
// inversion_count(values): i<jかつvalues[j]<values[i]の組数。O(n log(n+1))時間・O(n)領域。
// range_inversion_counts(values,ranges): 各半開区間[l,r)の転倒数を入力順で返す。
// ブロック幅をbとし、O((n log(n+1)+q log(q+1))+(n²/b+qb)log(n+1))時間・O(n+q)領域。
// n≤UINT32_MAX。比較は狭義弱順序。空区間・重複値を扱う。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_INVERSIONS_DEFINED
#define SIRKIT_RANGE_INVERSIONS_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void check_range_inversion_size(std::uint64_t size)
{
    if (size > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::length_error("range_inversion_counts: too many elements");
    }
}
} // namespace detail

template <class T>
std::vector<std::uint64_t> range_inversion_counts(
    const std::vector<T> &values, std::span<const std::pair<std::size_t, std::size_t>> ranges)
{
    const auto n = values.size();
    detail::check_range_inversion_size(n);
    std::vector<std::uint64_t> result(ranges.size());
    if (ranges.empty())
    {
        return result;
    }

    struct Query
    {
        std::size_t left, right, index, block;
    };
    const auto block_size = std::max<std::size_t>(1, static_cast<std::size_t>(n / std::sqrt(static_cast<long double>(ranges.size()))));
    std::vector<Query> queries;
    queries.reserve(ranges.size());
    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
        const auto [left, right] = ranges[i];
        if (left != right)
        {
            queries.push_back({left, right, i, left / block_size});
        }
    }
    if (queries.empty())
    {
        return result;
    }
    std::ranges::sort(queries, [](const Query &a, const Query &b)
                      {
                          if (a.block != b.block)
                          {
                              return a.block < b.block;
                          }
                          return a.block % 2 == 0 ? a.right < b.right : b.right < a.right;
                      });

    auto sorted = values;
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end(), [](const T &a, const T &b)
                             {
                                 return (a < b) == (b < a);
                             }),
                 sorted.end());
    std::vector<std::uint32_t> rank;
    rank.reserve(n);
    for (const auto &value : values)
    {
        rank.push_back(static_cast<std::uint32_t>(std::lower_bound(sorted.begin(), sorted.end(), value) - sorted.begin()));
    }

    std::vector<std::uint32_t> tree(sorted.size());
    const auto add = [&](std::size_t index, std::uint32_t delta)
    {
        for (; index < tree.size(); index |= index + 1)
        {
            tree[index] += delta;
        }
    };
    const auto count = [&](std::size_t end)
    {
        std::uint32_t total = 0;
        for (; end != 0; end &= end - 1)
        {
            total += tree[end - 1];
        }
        return total;
    };
    std::uint64_t inversions = 0;
    std::size_t left = 0, right = 0;
    for (const auto &[wanted_left, wanted_right, index, block] : queries)
    {
        while (left > wanted_left)
        {
            const auto r = rank[--left];
            inversions += count(r);
            add(r, 1);
        }
        while (right < wanted_right)
        {
            const auto r = rank[right];
            inversions += right - left - count(static_cast<std::size_t>(r) + 1);
            add(r, 1);
            ++right;
        }
        while (left < wanted_left)
        {
            const auto r = rank[left++];
            inversions -= count(r);
            add(r, std::numeric_limits<std::uint32_t>::max());
        }
        while (right > wanted_right)
        {
            const auto r = rank[--right];
            inversions -= right - left + 1 - count(static_cast<std::size_t>(r) + 1);
            add(r, std::numeric_limits<std::uint32_t>::max());
        }
        result[index] = inversions;
    }
    return result;
}

template <class T>
std::uint64_t inversion_count(const std::vector<T> &values)
{
    const std::pair<std::size_t, std::size_t> range{0, values.size()};
    return range_inversion_counts(values, {&range, 1}).front();
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
