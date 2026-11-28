// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Weighted_Monotone_Matching.cpp"
#include "../../../DataStructure/Segment_Tree.cpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
std::int64_t brute_force(const std::vector<sirkit::WeightedMatchEdge> &edges)
{
    std::int64_t best = 0;
    for (std::size_t mask = 1; mask < (std::size_t{1} << edges.size()); ++mask)
    {
        std::vector<sirkit::WeightedMatchEdge> selected;
        std::int64_t weight = 0;
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            if (((mask >> i) & 1) != 0)
            {
                selected.push_back(edges[i]);
                weight += edges[i].weight;
            }
        }
        std::ranges::sort(selected, {}, &sirkit::WeightedMatchEdge::left);
        const auto conflict = std::adjacent_find(selected.begin(), selected.end(), [](auto a, auto b) {
            return a.left >= b.left || a.right >= b.right;
        });
        if (conflict == selected.end())
        {
            best = std::max(best, weight);
        }
    }
    return best;
}
} // namespace

int main()
{
    std::mt19937 rng(231090);
    for (std::size_t trial = 0; trial < 290; ++trial)
    {
        std::vector<sirkit::WeightedMatchEdge> edges(trial % 12);
        for (auto &edge : edges)
        {
            edge = {rng() % 6, rng() % 6, static_cast<std::int64_t>(rng() % 21) - 10};
        }
        const auto actual = sirkit::maximum_weight_monotone_matching(edges);
        assert(actual.weight == brute_force(edges));
        std::int64_t total = 0;
        for (std::size_t i = 0; i < actual.indices.size(); ++i)
        {
            const auto &edge = edges.at(actual.indices[i]);
            if (i != 0)
            {
                const auto &previous = edges.at(actual.indices[i - 1]);
                assert(previous.left < edge.left && previous.right < edge.right);
            }
            total += edge.weight;
        }
        assert(total == actual.weight);
    }

    constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
    constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
    const std::array edges{sirkit::WeightedMatchEdge{0, 0, maximum}, sirkit::WeightedMatchEdge{1, 1, minimum}};
    assert(sirkit::maximum_weight_monotone_matching(edges).weight == maximum);
    bool overflow = false;
    try
    {
        const std::array bad{sirkit::WeightedMatchEdge{0, 0, maximum}, sirkit::WeightedMatchEdge{1, 1, 1}};
        static_cast<void>(sirkit::maximum_weight_monotone_matching(bad));
    }
    catch (const std::overflow_error &)
    {
        overflow = true;
    }
    assert(overflow);
}
