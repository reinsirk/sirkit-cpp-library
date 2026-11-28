// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Bipartite_Matching_Analysis.cpp"

#include <bit>
#include <cassert>
#include <limits>
#include <random>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;
using Role = sirkit::BipartiteEdgeRole;

void check(int left, int right, const std::vector<Edge> &edges)
{
    int best = -1;
    unsigned any_chosen = 0, all_chosen = 0, all_left = 0, all_right = 0;
    for (unsigned chosen = 0; chosen < (1U << edges.size()); ++chosen)
    {
        unsigned used_left = 0, used_right = 0;
        bool valid = true;
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            if ((chosen & (1U << i)) != 0)
            {
                const auto [u, v] = edges[i];
                if ((used_left & (1U << u)) || (used_right & (1U << v)))
                {
                    valid = false;
                    break;
                }
                used_left |= 1U << u;
                used_right |= 1U << v;
            }
        }
        if (!valid)
        {
            continue;
        }
        const int count = std::popcount(chosen);
        if (count > best)
        {
            best = count;
            any_chosen = all_chosen = chosen;
            all_left = used_left;
            all_right = used_right;
        }
        else if (count == best)
        {
            any_chosen |= chosen;
            all_chosen &= chosen;
            all_left &= used_left;
            all_right &= used_right;
        }
    }
    const auto result = sirkit::analyze_bipartite_matching(left, right, edges);
    assert(result.matching_size == best);
    assert(result.edge_role.size() == edges.size());
    assert(result.left_can_be_free.size() == static_cast<std::size_t>(left));
    assert(result.right_can_be_free.size() == static_cast<std::size_t>(right));
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto expected = (all_chosen & (1U << i))   ? Role::mandatory
                              : (any_chosen & (1U << i)) ? Role::optional
                                                         : Role::forbidden;
        assert(result.edge_role[i] == expected);
    }
    for (int u = 0; u < left; ++u)
    {
        assert(result.left_can_be_free[u] == ((all_left & (1U << u)) == 0));
    }
    for (int v = 0; v < right; ++v)
    {
        assert(result.right_can_be_free[v] == ((all_right & (1U << v)) == 0));
    }
}
} // namespace

int main()
{
    check(0, 0, {});
    check(0, 3, {});
    check(3, 0, {});
    check(2, 2, {{0, 0}, {0, 1}, {1, 1}});
    check(2, 2, {{0, 0}, {0, 0}, {0, 1}, {1, 0}, {1, 1}});
    check(3, 2, {{0, 0}, {1, 0}, {1, 1}, {2, 1}});
    check(2, 3, {{0, 0}, {0, 1}, {1, 1}, {1, 2}});

    std::mt19937 rng(17451744);
    for (int trial = 0; trial < 1050; ++trial)
    {
        const int left = 1 + rng() % 5, right = 1 + rng() % 5;
        const int count = rng() % 13;
        std::vector<Edge> edges;
        for (int i = 0; i < count; ++i)
        {
            edges.emplace_back(rng() % left, rng() % right);
        }
        check(left, right, edges);
    }

    int rejected = 0;
    for (const auto &[left, right] : {Edge{-1, 0}, Edge{0, -1}})
    {
        try
        {
            (void)sirkit::analyze_bipartite_matching(left, right, {});
        }
        catch (const std::invalid_argument &)
        {
            ++rejected;
        }
    }
    try
    {
        (void)sirkit::analyze_bipartite_matching(std::numeric_limits<int>::max(), 1, {});
    }
    catch (const std::length_error &)
    {
        ++rejected;
    }
    assert(rejected == 3);
#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<Edge> edges{{0, 1}};
    try
    {
        (void)sirkit::analyze_bipartite_matching(1, 1, edges);
    }
    catch (const std::out_of_range &)
    {
        ++rejected;
    }
    assert(rejected == 4);
#endif
}
