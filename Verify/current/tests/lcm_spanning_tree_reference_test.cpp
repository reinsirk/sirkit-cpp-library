// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/LCM_Spanning_Tree.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
std::uint64_t PrimCost(const std::vector<std::uint32_t> &labels)
{
    if (labels.empty())
    {
        return 0;
    }
    constexpr auto kInfinity = std::numeric_limits<std::uint64_t>::max();
    std::vector<std::uint64_t> best(labels.size(), kInfinity);
    std::vector<bool> used(labels.size());
    best[0] = 0;
    std::uint64_t cost = 0;
    for (std::size_t count = 0; count < labels.size(); ++count)
    {
        const auto next = static_cast<std::size_t>(std::ranges::min_element(best) - best.begin());
        used[next] = true;
        cost += best[next];
        best[next] = kInfinity;
        for (std::size_t v = 0; v < labels.size(); ++v)
        {
            if (!used[v])
            {
                const auto weight = std::lcm(std::uint64_t{labels[next]}, std::uint64_t{labels[v]});
                best[v] = std::min(best[v], weight);
            }
        }
    }
    return cost;
}

void Verify(const std::vector<std::uint32_t> &labels)
{
    const auto tree = sirkit::minimum_lcm_spanning_tree(labels);
    if (labels.empty())
    {
        assert(tree.empty());
        return;
    }
    assert(tree.size() == labels.size() - 1);
    std::vector<std::vector<int>> adjacent(labels.size());
    std::uint64_t cost = 0;
    for (const auto &edge : tree)
    {
        assert(static_cast<std::size_t>(edge.from) < labels.size());
        assert(static_cast<std::size_t>(edge.to) < labels.size());
        assert(edge.from != edge.to);
        assert(edge.weight == std::lcm(std::uint64_t{labels[edge.from]}, std::uint64_t{labels[edge.to]}));
        adjacent[edge.from].push_back(edge.to);
        adjacent[edge.to].push_back(edge.from);
        cost += edge.weight;
    }
    std::vector<bool> visited(labels.size());
    std::vector<int> pending{0};
    visited[0] = true;
    while (!pending.empty())
    {
        const auto u = pending.back();
        pending.pop_back();
        for (const auto v : adjacent[u])
        {
            if (!visited[v])
            {
                visited[v] = true;
                pending.push_back(v);
            }
        }
    }
    assert(std::ranges::all_of(visited, [](bool value)
                               {
                                   return value;
                               }));
    assert(cost == PrimCost(labels));
}
} // namespace

int main()
{
    Verify({});
    Verify({1});
    Verify({0xffff'ffffU});
    Verify({5, 5, 5});
    Verify({24, 12, 60});
    Verify({7, 9});
    Verify({1, 20, 28, 1});
    Verify({6, 9, 12, 18});
    Verify({21, 26, 35, 50, 6, 21, 55});
    Verify({1'000'000'000, 3'000'000'000, 4'000'000'000});
    Verify({2'000'000'000, 3'000'000'000, 4'000'000'000});
    Verify({54371, 40075, 150258, 164505, 5893, 123584, 57309, 117727, 159897, 94222});

    std::mt19937 random(102936);
    for (int trial = 0; trial < 1000; ++trial)
    {
        std::vector<std::uint32_t> labels(random() % 30 + 1);
        const auto scale = static_cast<std::uint32_t>(trial % 6 + 1);
        for (auto &label : labels)
        {
            label = scale * static_cast<std::uint32_t>(random() % (trial % 11 == 0 ? 6 : 201) + 1);
        }
        Verify(labels);
    }

    bool rejected = false;
    try
    {
        static_cast<void>(sirkit::minimum_lcm_spanning_tree(std::vector<std::uint32_t>{42, 0, 36}));
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected);
}
