#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_OPTIMAL_ADJACENT_MERGE_DEFINED
#define SIRKIT_OPTIMAL_ADJACENT_MERGE_DEFINED
struct AdjacentMergeResult
{
    std::uint64_t cost = 0;
    std::vector<std::array<std::size_t, 2>> merges;
};

namespace detail
{
struct AdjacentMergeItem
{
    std::uint64_t weight = 0;
    std::size_t index = 0;
};

struct AdjacentMergeAggregate
{
    std::array<AdjacentMergeItem, 2> first{};
    std::array<std::uint64_t, 2> last{};
    std::uint64_t maximum = 0;
    int count = 0; // 2で打ち切って保持する。
    bool has_candidate = false;

    static AdjacentMergeAggregate Singleton(std::uint64_t weight, std::size_t index) noexcept
    {
        return {{{{weight, index}, {}}}, {{weight, weight}}, weight, 1, false};
    }

    static AdjacentMergeAggregate Identity() noexcept
    {
        return {};
    }

    static AdjacentMergeAggregate Combine(AdjacentMergeAggregate left, AdjacentMergeAggregate right) noexcept
    {
        if (left.count == 0)
        {
            return right;
        }
        if (right.count == 0)
        {
            return left;
        }
        // 区間の境界をまたぐ長さ3の組だけを新たに調べる。
        const bool boundary_from_left = left.count == 2 && left.last[0] <= right.first[0].weight;
        const bool boundary_from_right = right.count == 2 && left.last[1] <= right.first[1].weight;
        AdjacentMergeAggregate merged;
        merged.first = {left.first[0], left.count == 2 ? left.first[1] : right.first[0]};
        merged.last = {right.count == 2 ? right.last[0] : left.last[1], right.last[1]};
        merged.maximum = std::max(left.maximum, right.maximum);
        merged.count = 2;
        merged.has_candidate = left.has_candidate || right.has_candidate || boundary_from_left || boundary_from_right;
        return merged;
    }

    static AdjacentMergeAggregate Map(bool, AdjacentMergeAggregate aggregate) noexcept
    {
        return aggregate;
    }

    static bool Compose(bool, bool) noexcept
    {
        return false;
    }

    static bool NoAction() noexcept
    {
        return false;
    }
};

inline std::uint64_t CheckedAdjacentMergeAdd(std::uint64_t left, std::uint64_t right)
{
    if (right > std::numeric_limits<std::uint64_t>::max() - left)
    {
        throw std::overflow_error("optimal_adjacent_merge: cost exceeds uint64_t");
    }
    return left + right;
}

inline AdjacentMergeResult OptimalAdjacentMerge(std::span<const std::uint64_t> weights)
{
    AdjacentMergeResult result;
    const auto n = weights.size();
    if (n < 2)
    {
        return result;
    }
    using Aggregate = AdjacentMergeAggregate;
    using Sequence = lazy_implicit_treap<Aggregate, Aggregate::Combine, Aggregate::Identity, bool,
                                         Aggregate::Map, Aggregate::Compose, Aggregate::NoAction>;
    auto sequence = [&]
    {
        std::vector<Aggregate> items;
        items.reserve(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            items.push_back(Aggregate::Singleton(weights[i], i));
        }
        return Sequence(items);
    }();
    result.merges.reserve(n - 1);
    // いったん順序を崩して併合し、最適木の葉の深さを求める。
    while (sequence.size() > 1)
    {
        const int start = sequence.max_right(0, [](const Aggregate &range)
                                             {
                                                 return !range.has_candidate;
                                             }) -
                          2;
        const auto pair = sequence.erase_prod(start, start + 2).first;
        const auto weight = CheckedAdjacentMergeAdd(pair[0].weight, pair[1].weight);
        result.cost = CheckedAdjacentMergeAdd(result.cost, weight);
        const int place = sequence.min_left(start, [weight](const Aggregate &range)
                                            {
                                                return range.count == 0 || range.maximum < weight;
                                            });
        sequence.insert_at(place, Aggregate::Singleton(weight, n + result.merges.size()));
        result.merges.push_back({pair[0].index, pair[1].index});
    }
    std::vector<int> depths(n + result.merges.size());
    for (std::size_t i = result.merges.size(); i != 0; --i)
    {
        const auto [left, right] = result.merges[i - 1];
        depths[left] = depths[right] = depths[n + i - 1] + 1;
    }
    result.merges.clear();
    std::vector<std::pair<std::size_t, int>> stack;
    stack.reserve(n);
    // 元の葉順で同じ深さの隣接部分木をつなぎ、合法な併合順へ戻す。
    for (std::size_t i = 0; i < n; ++i)
    {
        stack.emplace_back(i, depths[i]);
        while (stack.size() > 1 && stack.back().second == stack[stack.size() - 2].second)
        {
            const auto [right, depth] = stack.back();
            stack.pop_back();
            const auto left = stack.back().first;
            stack.back() = {n + result.merges.size(), depth - 1};
            result.merges.push_back({left, right});
        }
    }
    return result;
}
}

inline AdjacentMergeResult optimal_adjacent_merge(std::span<const std::uint64_t> weights)
{
    return detail::OptimalAdjacentMerge(weights);
}
#endif
