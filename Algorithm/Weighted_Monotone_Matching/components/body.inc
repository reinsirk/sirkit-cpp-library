#include <cstdint>
#include <numeric>
#include <span>

#ifndef SIRKIT_WEIGHTED_MONOTONE_MATCHING_DEFINED
#define SIRKIT_WEIGHTED_MONOTONE_MATCHING_DEFINED

struct WeightedMatchEdge
{
    std::size_t left, right;
    std::int64_t weight;
};

struct WeightedMonotoneMatching
{
    std::int64_t weight;
    std::vector<std::size_t> indices;
};

namespace detail
{
inline constexpr auto kNoWeightedMatchEdge = std::numeric_limits<std::size_t>::max();

struct WeightedChainEnd
{
    std::int64_t weight = 0;
    std::size_t index = kNoWeightedMatchEdge;
};

inline constexpr WeightedChainEnd choose_heavier_chain(WeightedChainEnd first, WeightedChainEnd second) noexcept
{
    return first.weight < second.weight ? second : first;
}

inline constexpr WeightedChainEnd empty_weighted_chain() noexcept
{
    return {};
}
} // namespace detail

inline WeightedMonotoneMatching maximum_weight_monotone_matching(std::span<const WeightedMatchEdge> edges)
{
    const auto n = edges.size();
    std::vector<std::size_t> right_coordinates;
    right_coordinates.reserve(n);
    for (const auto &edge : edges)
    {
        right_coordinates.push_back(edge.right);
    }
    std::sort(right_coordinates.begin(), right_coordinates.end());
    right_coordinates.erase(std::unique(right_coordinates.begin(), right_coordinates.end()), right_coordinates.end());
    const auto width = detail::CheckedSegmentArrayCount(right_coordinates.size(), "weighted_monotone_matching: too many positions");
    segtree<detail::WeightedChainEnd, detail::choose_heavier_chain, detail::empty_weighted_chain> best(width);

    std::vector<std::size_t> order(n), previous(n);
    std::iota(order.begin(), order.end(), std::size_t{0});
    // 左位置が等しい辺は右位置の降順に処理し、同じ左位置から2本取ることを防ぐ。
    std::sort(order.begin(), order.end(), [&](std::size_t i, std::size_t j) {
        if (edges[i].left != edges[j].left)
        {
            return edges[i].left < edges[j].left;
        }
        return edges[i].right > edges[j].right;
    });
    for (const auto id : order)
    {
        const auto &edge = edges[id];
        const auto r = std::lower_bound(right_coordinates.begin(), right_coordinates.end(), edge.right);
        const auto rank = static_cast<int>(r - right_coordinates.begin());
        const auto parent = best.prod(0, rank);
        if (edge.weight > std::numeric_limits<std::int64_t>::max() - parent.weight)
        {
            throw std::overflow_error("weighted_monotone_matching: selected weight overflows");
        }
        const auto weight = parent.weight + edge.weight;
        if (weight > best.get(rank).weight)
        {
            best.set(rank, {weight, id});
            previous[id] = parent.index;
        }
    }
    const auto end = best.all_prod();
    WeightedMonotoneMatching result{end.weight, {}};
    for (auto id = end.index; id != detail::kNoWeightedMatchEdge; id = previous[id])
    {
        result.indices.push_back(id);
    }
    std::reverse(result.indices.begin(), result.indices.end());
    return result;
}
#endif
