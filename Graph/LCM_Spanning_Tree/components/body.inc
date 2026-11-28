#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_LCM_SPANNING_TREE_DEFINED
#define SIRKIT_LCM_SPANNING_TREE_DEFINED

struct LcmSpanningTreeEdge
{
    int from;
    int to;
    std::uint64_t weight;
};

[[nodiscard]] inline std::vector<LcmSpanningTreeEdge> minimum_lcm_spanning_tree(std::span<const std::uint32_t> labels)
{
    if (labels.empty())
    {
        return {};
    }
    const auto [minimum, maximum] = std::ranges::minmax_element(labels);
    if (*minimum == 0)
    {
        throw std::invalid_argument("minimum_lcm_spanning_tree: zero label");
    }
    UnionFind components(labels.size());
    std::uint32_t scale = *minimum;
    for (auto label : labels)
    {
        scale = std::gcd(scale, label);
        if (scale == 1)
        {
            break;
        }
    }
    std::vector<LcmSpanningTreeEdge> result;
    result.reserve(labels.size() - 1);
    if (scale == *minimum)
    {
        const auto center = minimum - labels.begin();
        for (std::size_t v = 0; v < labels.size(); ++v)
        {
            if (v != static_cast<std::size_t>(center))
            {
                result.push_back({static_cast<int>(center), static_cast<int>(v), labels[v]});
            }
        }
        return result;
    }

    const std::size_t bound = *maximum / scale;
    std::vector<int> representative(bound + 1, -1);
    for (std::size_t v = 0; v < labels.size(); ++v)
    {
        const auto label = labels[v];
        auto &first = representative[label / scale];
        if (first == -1)
        {
            first = static_cast<int>(v);
        }
        else
        {
            components.unite(first, static_cast<int>(v));
            result.push_back({first, static_cast<int>(v), label});
        }
    }

    std::vector<LcmSpanningTreeEdge> candidates;
    for (std::size_t divisor = 1; divisor <= bound; ++divisor)
    {
        std::size_t center = 0;
        for (auto label = divisor; label <= bound; label += divisor)
        {
            const auto vertex = representative[label];
            if (vertex == -1)
            {
                continue;
            }
            if (center != 0)
            {
                // 最大公約数がdivisorの元の辺は、その倍数で最小のラベルを経由して置き換えられる。
                const auto weight = std::uint64_t{center / divisor} * label * scale;
                candidates.push_back({representative[center], vertex, weight});
            }
            else
            {
                center = label;
            }
        }
    }
    std::ranges::sort(candidates, {}, &LcmSpanningTreeEdge::weight);
    for (auto edge = candidates.begin(); result.size() + 1 < labels.size(); ++edge)
    {
        if (components.merge(edge->from, edge->to))
        {
            result.push_back(*edge); // 過大評価した辺は、同じ両端を結ぶ軽い経路が先にできるため採用されない。
        }
    }
    return result;
}

#endif
