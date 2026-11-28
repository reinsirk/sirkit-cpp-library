// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Metric.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
using Table = std::vector<std::vector<std::uint64_t>>;
using Edge = std::tuple<int, int, std::uint64_t>;

Table distances(int n, const std::set<Edge> &edges)
{
    std::vector<std::vector<std::pair<int, std::uint64_t>>> adjacency(n);
    for (const auto &[u, v, weight] : edges)
    {
        adjacency[u].emplace_back(v, weight);
        adjacency[v].emplace_back(u, weight);
    }
    Table answer(n, std::vector<std::uint64_t>(n));
    for (int start = 0; start < n; ++start)
    {
        std::vector<int> pending{start};
        std::vector<bool> found(n);
        found[start] = true;
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const int u = pending[i];
            for (const auto &[v, weight] : adjacency[u])
            {
                if (!found[v])
                {
                    found[v] = true;
                    pending.push_back(v);
                    answer[start][v] = answer[start][u] + weight;
                }
            }
        }
    }
    return answer;
}

std::optional<std::set<Edge>> irreducible_pairs(const Table &values)
{
    const int n = static_cast<int>(values.size());
    std::set<Edge> tree;
    for (int u = 0; u < n; ++u)
    {
        if (values[u][u] != 0)
        {
            return std::nullopt;
        }
        for (int v = u + 1; v < n; ++v)
        {
            if (values[u][v] == 0 || values[u][v] != values[v][u])
            {
                return std::nullopt;
            }
            bool direct = true;
            for (int middle = 0; middle < n; ++middle)
            {
                if (middle != u && middle != v && values[u][v] == values[u][middle] + values[middle][v])
                {
                    direct = false;
                    break;
                }
            }
            if (direct)
            {
                tree.emplace(u, v, values[u][v]);
            }
        }
    }
    if (tree.size() != (n == 0 ? 0U : static_cast<unsigned>(n - 1)) || distances(n, tree) != values)
    {
        return std::nullopt;
    }
    return tree;
}

void check_small(const Table &matrix)
{
    const int n = static_cast<int>(matrix.size());
    const auto result = sirkit::reconstruct_tree_metric(n, [&](int u, int v)
                                                        {
                                                            return matrix[u][v];
                                                        });
    const auto expected = irreducible_pairs(matrix);
    assert(result.has_value() == expected.has_value());
    if (result)
    {
        std::set<Edge> edges;
        for (const auto &[u, v, weight] : *result)
        {
            assert(0 <= u && u < n && 0 <= v && v < n && u != v);
            assert(edges.emplace(std::min(u, v), std::max(u, v), weight).second);
        }
        assert(edges == *expected);
    }
}
} // namespace

int main()
{
    check_small({});
    check_small({{0}});
    check_small({{2}});
    check_small({{0, 0}, {0, 0}});
    check_small({{0, 1}, {2, 0}});
    check_small({{0, 2, 5}, {2, 0, 3}, {5, 3, 0}});
    check_small({{0, 2, 5}, {2, 0, 3}, {4, 3, 0}});
    assert(!sirkit::reconstruct_tree_metric(2, [](int u, int v)
                                            {
                                                return u == v ? 0 : -1;
                                            }));
    assert(!sirkit::reconstruct_tree_metric(2, [](int u, int v)
                                            {
                                                return u == v ? 0 : (u == 0 ? 1 : -1);
                                            }));
    assert(sirkit::reconstruct_tree_metric(2, [](int u, int v)
                                           {
                                               return u == v ? 0 : 1;
                                           }));

    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const Table edge_max{{0, kMax}, {kMax, 0}};
    const auto maximum = sirkit::reconstruct_tree_metric(2, [&](int u, int v)
                                                         {
                                                             return edge_max[u][v];
                                                         });
    assert(maximum && maximum->size() == 1 && maximum->front().weight == kMax);
    const Table route_max{{0, kMax - 1, kMax}, {kMax - 1, 0, 1}, {kMax, 1, 0}};
    const auto route = sirkit::reconstruct_tree_metric(3, [&](int u, int v)
                                                       {
                                                           return route_max[u][v];
                                                       });
    assert(route && route->size() == 2);
    const Table overflow{{0, kMax, kMax}, {kMax, 0, kMax}, {kMax, kMax, 0}};
    assert(!sirkit::reconstruct_tree_metric(3, [&](int u, int v)
                                            {
                                                return overflow[u][v];
                                            }));

    std::mt19937 random(4515258);
    for (int trial = 0; trial < 810; ++trial)
    {
        const int n = static_cast<int>(random() % 12) + 1;
        std::set<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            edges.emplace(static_cast<int>(random() % v), v, random() % 20 + 1);
        }
        auto table = distances(n, edges);
        check_small(table);
        const int u = static_cast<int>(random() % n);
        const int v = static_cast<int>(random() % n);
        table[u][v] += random() % 5 + 1;
        if (trial % 2 == 0)
        {
            table[v][u] = table[u][v];
        }
        check_small(table);

        for (int i = 0; i < n; ++i)
        {
            table[i][i] = 0;
            for (int j = i + 1; j < n; ++j)
            {
                table[i][j] = table[j][i] = random() % 50 + 1;
            }
        }
        check_small(table);
    }

    bool caught = false;
    try
    {
        (void)sirkit::reconstruct_tree_metric(-1, [](int, int)
                                              {
                                                  return 0;
                                              });
    }
    catch (const std::length_error &)
    {
        caught = true;
    }
    assert(caught);
}
