// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Edge_Color_Path_Counts.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::TreeColorEdge;

void check(int n, const std::vector<Edge> &edges)
{
    std::vector<std::vector<std::pair<int, std::int64_t>>> graph(n);
    for (const auto &[u, v, color] : edges)
    {
        graph[u].emplace_back(v, color);
        graph[v].emplace_back(u, color);
    }
    std::uint64_t monochromatic = 0, bichromatic = 0;
    for (int root = 0; root < n; ++root)
    {
        auto search = [&](auto &&self, int v, int previous, const std::set<std::int64_t> &path) -> void
        {
            if (root < v)
            {
                monochromatic += path.size() == 1;
                bichromatic += path.size() == 2;
            }
            for (const auto &[u, label] : graph[v])
            {
                if (u != previous)
                {
                    auto next = path;
                    next.insert(label);
                    self(self, u, v, next);
                }
            }
        };
        search(search, root, -1, {});
    }
    const auto actual = sirkit::tree_edge_color_path_counts(n, edges);
    assert(actual.one_color == monochromatic && actual.two_colors == bichromatic);
}

template <typename Error, typename Action>
void expect_error(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Error &)
    {
        caught = true;
    }
    assert(caught);
}
} // namespace

int main()
{
    check(0, {});
    check(1, {});
    check(2, {{1, 0, 0}});
#ifdef SKIP_RUNTIME_CHECKS
    const sirkit::CentroidDecomposition centroid(sirkit::Graph(1));
    assert(centroid.depth(0) == 0);
#endif
    check(8, {{0, 1, -2}, {1, 2, -2}, {2, 3, -3}, {3, 4, -2}, {4, 5, -3}, {5, 6, 2}, {6, 7, -2}});
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    check(11, {{0, 1, kMin}, {1, 2, kMax}, {2, 3, kMin}, {0, 4, kMax}, {4, 5, kMin}, {5, 6, kMax}, {0, 7, kMax}, {7, 8, kMin}, {0, 9, 0}, {0, 10, -1}});

    std::mt19937 random(100221371);
    for (int trial = 0; trial < 860; ++trial)
    {
        const int n = static_cast<int>(random() % 28) + 1;
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            const int parent = static_cast<int>(random() % v);
            const auto color = static_cast<int>(random() % (1 + trial % 9)) - 4;
            if (trial % 2 == 0)
            {
                edges.emplace_back(v, parent, color);
            }
            else
            {
                edges.emplace_back(parent, v, color);
            }
        }
        std::shuffle(edges.begin(), edges.end(), random);
        check(n, edges);
    }

    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::tree_edge_color_path_counts(-1, {});
                                    });
    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::tree_edge_color_path_counts(2, {});
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int bad : {-1, 2})
    {
        const std::vector<Edge> from{{bad, 0, 1}}, to{{0, bad, 1}};
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::tree_edge_color_path_counts(2, from);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::tree_edge_color_path_counts(2, to);
                                        });
    }
    const std::vector<Edge> self{{0, 0, 9}};
    const std::vector<Edge> cycle{{0, 1, 9}, {1, 2, 9}, {2, 0, 9}};
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)sirkit::tree_edge_color_path_counts(2, self);
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)sirkit::tree_edge_color_path_counts(4, cycle);
                                        });
#endif
}
