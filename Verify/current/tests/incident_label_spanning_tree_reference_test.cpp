// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Incident_Label_Spanning_Tree.cpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Edge = sirkit::LabeledSpanningEdge<int>;

bool accepted(const std::vector<int> &needs, const std::vector<Edge> &edges,
              const std::vector<std::size_t> &chosen)
{
    if (needs.empty())
    {
        return chosen.empty();
    }
    if (chosen.size() + 1 != needs.size())
    {
        return false;
    }
    std::vector<bool> agrees(needs.size()), used(edges.size());
    std::vector<std::vector<int>> adjacent(needs.size());
    for (const auto id : chosen)
    {
        if (id >= edges.size() || used[id])
        {
            return false;
        }
        used[id] = true;
        const auto &[u, v, color] = edges[id];
        agrees[u] = agrees[u] || (color == needs[u]);
        agrees[v] = agrees[v] || (color == needs[v]);
        adjacent[u].push_back(v);
        adjacent[v].push_back(u);
    }
    if (std::ranges::find(agrees, false) != agrees.end())
    {
        return false;
    }
    std::vector<bool> reached(needs.size());
    reached[0] = true;
    std::vector<int> pending{0};
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        for (const int v : adjacent[pending[i]])
        {
            if (!reached[v])
            {
                reached[v] = true;
                pending.push_back(v);
            }
        }
    }
    return pending.size() == needs.size();
}

void check(const std::vector<int> &needs, const std::vector<Edge> &edges)
{
    bool possible = needs.empty();
    for (std::uint32_t mask = 0; mask < (1U << edges.size()); ++mask)
    {
        if (static_cast<std::size_t>(std::popcount(mask)) + 1 != needs.size())
        {
            continue;
        }
        std::vector<std::size_t> chosen;
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            if ((mask >> i) & 1U)
            {
                chosen.push_back(i);
            }
        }
        possible = possible || accepted(needs, edges, chosen);
    }

    const auto result = sirkit::incident_label_spanning_tree<int>(needs, edges);
    assert(result.has_value() == possible);
    if (result)
    {
        assert(accepted(needs, edges, *result));
    }
}
} // namespace

int main()
{
    check({}, {});
    check({4}, {{0, 0, 4}});
    check({1, 2}, {{0, 1, 1}, {0, 1, 2}});
    check({1, 1, 2}, {{0, 1, 1}, {1, 2, 2}, {0, 2, 2}});
    check({1, 1, 2, 3, 3}, {{0, 1, 1}, {1, 2, 2}, {2, 3, 3}, {3, 4, 3}, {2, 4, 7}});
    check({0, 0, 1, 1}, {{0, 1, 0}, {2, 3, 1}});
    check({0, 0, 1, 1}, {{0, 1, 0}, {2, 3, 1}, {1, 2, 5}});
    check({2, 2, 2, 2}, {{0, 1, 2}, {1, 2, 2}, {2, 0, 2}, {0, 3, 2}, {0, 0, 2}, {0, 3, 2}});

    const std::array required{1, 2};
    for (const auto &[u, v] : {std::pair{-1, 0}, std::pair{2, 0}, std::pair{0, -1}, std::pair{0, 2}})
    {
        bool caught = false;
        try
        {
            const std::array bad{Edge{u, v, 1}};
            (void)sirkit::incident_label_spanning_tree<int>(required, bad);
        }
        catch (const std::out_of_range &)
        {
            caught = true;
        }
        assert(caught);
    }

    std::mt19937 random(64012094);
    for (int trial = 0; trial < 950; ++trial)
    {
        const int n = static_cast<int>(random() % 8) + 1;
        std::vector<int> needs(n);
        for (auto &color : needs)
        {
            color = static_cast<int>(random() % 3);
        }
        std::vector<Edge> edges(random() % 13);
        for (auto &edge : edges)
        {
            edge = {static_cast<int>(random() % n), static_cast<int>(random() % n), static_cast<int>(random() % 3)};
        }
        check(needs, edges);
    }
}
