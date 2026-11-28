// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Algorithm/Adjacent_Swap_Distance.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/adjacent_swap_distance.md
//
// API一覧
// adjacent_swap_permutation(from,to,compare={}): from[i]に対応するtoの位置を添字iの順に返す。
// 同じ値の出現は先頭から順に対応させる。添字は0始まり。
// adjacent_swap_distance(from,to,compare={}): 隣接する2要素の交換でfromをtoへ変える最小回数。
// 変換できない場合はいずれもnullopt。compareで互いに小さくない値を同値と扱う。
// std::size・添字アクセスに対応する列で使用可。長さnはUINT32_MAX以下。O((n+1)log(n+2))時間・O(n+1)領域。
//
// [同梱の転倒数]
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
#include <functional>
#include <iterator>
#include <numeric>
#include <optional>


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
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > n)
        {
            throw std::out_of_range("range_inversion_counts: invalid interval");
        }
#endif
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

#ifndef SIRKIT_ADJACENT_SWAP_DISTANCE_DEFINED
#define SIRKIT_ADJACENT_SWAP_DISTANCE_DEFINED

template<class From, class To, class Compare = std::less<>>
std::optional<std::vector<std::size_t>> adjacent_swap_permutation(const From &from, const To &to, Compare compare = {})
{
    const auto n = static_cast<std::size_t>(std::size(from));
    if (n != static_cast<std::size_t>(std::size(to)))
    {
        return std::nullopt;
    }
    detail::check_range_inversion_size(n);
    const auto sorted_indices = [&](const auto &values) {
        std::vector<std::size_t> order(n);
        std::iota(order.begin(), order.end(), std::size_t{0});
        std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
            if (std::invoke(compare, values[a], values[b]))
            {
                return true;
            }
            if (std::invoke(compare, values[b], values[a]))
            {
                return false;
            }
            return a < b;
        });
        return order;
    };
    const auto sources = sorted_indices(from);
    const auto targets = sorted_indices(to);
    std::vector<std::size_t> permutation(n);
    for (std::size_t rank = 0; rank < n; ++rank)
    {
        const auto first = sources[rank];
        const auto second = targets[rank];
        if (std::invoke(compare, from[first], to[second]) || std::invoke(compare, to[second], from[first]))
        {
            return std::nullopt;
        }
        permutation[first] = second;
    }
    return permutation;
}

template<class From, class To, class Compare = std::less<>>
std::optional<std::uint64_t> adjacent_swap_distance(const From &from, const To &to, Compare compare = {})
{
    const auto permutation = adjacent_swap_permutation(from, to, std::move(compare));
    if (!permutation)
    {
        return std::nullopt;
    }
    // 同値な要素どうしの順序を保った対応なら、余分な交差がない。
    return inversion_count(*permutation);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
