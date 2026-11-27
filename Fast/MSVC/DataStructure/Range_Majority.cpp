// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Range_Majority.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_majority.md
//
// API一覧
// RangeMajoritySet<T>{position,value}: 指定位置の値を置き換える。
// RangeMajorityQuery{left,right}: 半開区間[left,right)の過半数を問い合わせる。
// RangeMajorityOperation<T>: 上の2操作を保持するstd::variant。
// range_majority_queries(values,operations): 操作を順に適用し、問い合わせの答えだけを順番に返す。
// 区間長の半分を超える値を返し、空区間や該当する値がなければnullopt。元のvaluesは変更しない。
// 要素数n、操作数q、更新数u。n+u≤2^32−1。時間O((n+q)log(n+u+2))、領域O(n+q)。
// 計算量は値のコピー・比較をO(1)と数える。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_MAJORITY_DEFINED
#define SIRKIT_RANGE_MAJORITY_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void CheckRangeMajorityCapacity(std::size_t n, std::size_t updates, std::size_t location_limit,
                                       std::size_t candidate_limit)
{
    if (n > std::min(location_limit, candidate_limit / 2))
    {
        throw std::length_error("range_majority_queries: sequence too large");
    }
    if (updates > location_limit - n)
    {
        throw std::length_error("range_majority_queries: too many updates");
    }
}
} // namespace detail

template <class T>
struct RangeMajoritySet
{
    std::size_t position;
    T value;
};

struct RangeMajorityQuery
{
    std::size_t left, right;
};

template <class T>
using RangeMajorityOperation = std::variant<RangeMajoritySet<T>, RangeMajorityQuery>;

template <class T>
std::vector<std::optional<T>> range_majority_queries(
    const std::vector<T> &values, std::span<const std::type_identity_t<RangeMajorityOperation<T>>> operations)
{
    using Index = std::uint32_t;
    using Location = std::pair<T, Index>;
    struct Vote
    {
        Index group = 0;
        Index balance = 0;
    };
    const auto n = values.size();
    constexpr auto kMaxIndex = std::numeric_limits<Index>::max();
    std::vector<Location> locations;
    std::vector<Vote> candidates;
    const auto capacity = std::min<std::size_t>(kMaxIndex, locations.max_size());

    std::size_t updates = 0;
    for (const auto &operation : operations)
    {
        updates += std::holds_alternative<RangeMajoritySet<T>>(operation);
    }
    detail::CheckRangeMajorityCapacity(n, updates, capacity, candidates.max_size());
    locations.reserve(n + updates);
    for (std::size_t i = 0; i < n; ++i)
    {
        locations.emplace_back(values[i], static_cast<Index>(i));
    }
    for (const auto &operation : operations)
    {
        if (const auto *set = std::get_if<RangeMajoritySet<T>>(&operation))
        {
            locations.emplace_back(set->value, static_cast<Index>(set->position));
        }
    }
    std::sort(locations.begin(), locations.end());
    locations.erase(std::unique(locations.begin(), locations.end()), locations.end());

    const auto combine = [](Vote a, Vote b) -> Vote
    {
        if (a.group == b.group)
        {
            return {a.group, a.balance + b.balance};
        }
        return a.balance > b.balance ? Vote{a.group, a.balance - b.balance} : Vote{b.group, b.balance - a.balance};
    };

    std::vector<Index> current(n), group_of(locations.size()), counts(locations.size());
    std::vector<Index> group_begin;
    candidates.resize(2 * n);
    for (std::size_t i = 0; i < locations.size(); ++i)
    {
        const auto &[value, position] = locations[i];
        if (i == 0 || locations[i - 1].first != value)
        {
            group_begin.push_back(static_cast<Index>(i));
        }
        const auto group = static_cast<Index>(group_begin.size() - 1);
        group_of[i] = group;
        if (value == values[position])
        {
            current[position] = static_cast<Index>(i);
            candidates[n + position] = {group, 1};
            counts[i] = 1;
        }
    }
    group_begin.push_back(static_cast<Index>(locations.size()));

    for (auto i = n; i > 1;)
    {
        --i;
        candidates[i] = combine(candidates[2 * i], candidates[2 * i + 1]);
    }
    for (std::size_t i = 0; i < counts.size(); ++i)
    {
        const auto parent = i | (i + 1);
        if (parent < counts.size())
        {
            counts[parent] += counts[i];
        }
    }

    const auto modify = [&]<bool Add>(std::size_t index)
    {
        for (; index < counts.size(); index |= index + 1)
        {
            if constexpr (Add)
            {
                ++counts[index];
            }
            else
            {
                --counts[index];
            }
        }
    };
    const auto prefix_count = [&](std::size_t end)
    {
        Index count = 0;
        for (; end != 0; end &= end - 1)
        {
            count += counts[end - 1];
        }
        return count;
    };

    std::vector<std::optional<T>> result;
    result.reserve(operations.size() - updates);
    for (const auto &operation : operations)
    {
        if (const auto *set = std::get_if<RangeMajoritySet<T>>(&operation))
        {
            auto &slot = current[set->position];
            if (locations[slot].first == set->value)
            {
                continue;
            }
            // 同じ値・位置の座標を使い回し、現在の値の座標だけを有効にする。
            const Location key{set->value, static_cast<Index>(set->position)};
            const auto replacement = static_cast<Index>(std::lower_bound(locations.begin(), locations.end(), key) -
                                                        locations.begin());
            modify.template operator()<false>(slot);
            modify.template operator()<true>(replacement);
            slot = replacement;

            auto i = n + set->position;
            candidates[i] = {group_of[slot], 1};
            for (i /= 2; i != 0; i /= 2)
            {
                candidates[i] = combine(candidates[2 * i], candidates[2 * i + 1]);
            }
            continue;
        }

        const auto [left, right] = std::get<RangeMajorityQuery>(operation);
        Vote candidate;
        // 異なる値を対にして消しても、過半数の値は消え切らない。
        for (auto a = left + n, b = right + n; a < b; a /= 2, b /= 2)
        {
            if (a % 2 != 0)
            {
                candidate = combine(candidate, candidates[a++]);
            }
            if (b % 2 != 0)
            {
                candidate = combine(candidate, candidates[--b]);
            }
        }
        if (candidate.balance == 0)
        {
            result.emplace_back(std::nullopt);
            continue;
        }

        const auto begin = locations.begin() + group_begin[candidate.group];
        const auto end = locations.begin() + group_begin[static_cast<std::size_t>(candidate.group) + 1];
        const auto half = (right - left) / 2;
        if (candidate.balance <= half)
        {
            const auto position_less = [](const Location &coordinate, std::size_t position)
            {
                return coordinate.second < position;
            };
            const auto a = static_cast<std::size_t>(std::lower_bound(begin, end, left, position_less) - locations.begin());
            const auto b = static_cast<std::size_t>(std::lower_bound(begin, end, right, position_less) - locations.begin());
            if (prefix_count(b) - prefix_count(a) <= half)
            {
                result.emplace_back(std::nullopt);
                continue;
            }
        }
        result.emplace_back(begin->first);
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
