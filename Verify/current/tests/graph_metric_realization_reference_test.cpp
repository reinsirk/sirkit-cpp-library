// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Graph_Metric_Realization.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Entry = std::optional<std::uint64_t>;
using Matrix = std::vector<std::vector<Entry>>;
using Edge = sirkit::MetricGraphEdge;

Matrix closure(int n, const std::vector<Edge> &edges, bool directed)
{
    Matrix distance(n, std::vector<Entry>(n));
    for (int v = 0; v < n; ++v)
    {
        distance[v][v] = 0;
    }
    for (const auto &[u, v, cost] : edges)
    {
        distance[u][v] = cost;
        if (!directed)
        {
            distance[v][u] = cost;
        }
    }
    for (int k = 0; k < n; ++k)
    {
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                if (distance[i][k] && distance[k][j])
                {
                    const auto candidate = *distance[i][k] + *distance[k][j];
                    if (!distance[i][j] || candidate < *distance[i][j])
                    {
                        distance[i][j] = candidate;
                    }
                }
            }
        }
    }
    return distance;
}

auto reconstruct(const Matrix &dist, bool directed)
{
    return sirkit::realize_graph_metric(static_cast<int>(dist.size()), [&](int u, int v)
                                        {
                                            return dist[u][v];
                                        },
                                        directed);
}

void check_reference(const Matrix &matrix, bool directed, bool exhaustive)
{
    const int n = static_cast<int>(matrix.size());
    const auto result = reconstruct(matrix, directed);
    assert(result);
    assert(closure(n, result->edges, directed) == matrix);
    std::uint64_t cost = 0;
    for (const auto &edge : result->edges)
    {
        cost += edge.weight;
    }
    assert(result->total_weight == cost);
    for (std::size_t skip = 0; skip < result->edges.size(); ++skip)
    {
        auto copy = result->edges;
        copy.erase(copy.begin() + static_cast<std::ptrdiff_t>(skip));
        assert(closure(n, copy, directed) != matrix);
    }
    if (!exhaustive)
    {
        return;
    }

    std::vector<Edge> all;
    for (int u = 0; u < n; ++u)
    {
        for (int v = directed ? 0 : u + 1; v < n; ++v)
        {
            if (u != v && matrix[u][v])
            {
                all.push_back({u, v, *matrix[u][v]});
            }
        }
    }
    auto brute = cost + 1;
    for (std::size_t mask = 0; mask < (std::size_t{1} << all.size()); ++mask)
    {
        std::vector<Edge> selected;
        std::uint64_t sum = 0;
        for (std::size_t i = 0; i < all.size(); ++i)
        {
            if ((mask >> i) & 1U)
            {
                selected.push_back(all[i]);
                sum += all[i].weight;
            }
        }
        if (sum < brute && closure(n, selected, directed) == matrix)
        {
            brute = sum;
        }
    }
    assert(brute == cost);
}
} // namespace

int main()
{
    assert(sirkit::detail::graph_metric_area(0, 0) == 0);
    assert(sirkit::detail::graph_metric_area(4, 16) == 16);
    bool caught = false;
    try
    {
        (void)sirkit::detail::graph_metric_area(4, 15);
    }
    catch (const std::length_error &)
    {
        caught = true;
    }
    assert(caught);
    caught = false;
    try
    {
        (void)sirkit::realize_graph_metric(-1, [](int, int)
                                           {
                                               return 0;
                                           });
    }
    catch (const std::length_error &)
    {
        caught = true;
    }
    assert(caught);

    check_reference({}, false, true);
    check_reference({{0}}, true, true);
    check_reference({{0, 1, 3}, {1, 0, 2}, {3, 2, 0}}, false, true);
    check_reference({{0, 5, 10}, {std::nullopt, 0, 5}, {std::nullopt, std::nullopt, 0}}, true, true);
    assert(!reconstruct({{1}}, false));
    assert(!reconstruct({{std::nullopt}}, true));
    assert(!reconstruct({{0, 0}, {0, 0}}, false));
    assert(!reconstruct({{0, 1}, {2, 0}}, false));
    assert(!reconstruct({{0, 1, 3}, {1, 0, 1}, {3, 1, 0}}, false));
    assert(!reconstruct({{0, 1, std::nullopt}, {std::nullopt, 0, 1}, {std::nullopt, std::nullopt, 0}}, true));
    assert(!sirkit::realize_graph_metric(2, [](int u, int v)
                                         {
                                             return u == v ? 0 : -1;
                                         }));
    assert(sirkit::realize_graph_metric(2, [](int u, int v)
                                        {
                                            return u == v ? 0 : 1;
                                        }));

    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const auto exact = reconstruct({{0, kMax - 1, kMax}, {std::nullopt, 0, 1}, {std::nullopt, std::nullopt, 0}}, true);
    assert(exact && exact->edges.size() == 2 && exact->total_weight == kMax);
    const auto large = reconstruct({{0, kMax, kMax}, {kMax, 0, kMax}, {kMax, kMax, 0}}, false);
    assert(large && large->edges.size() == 3 && !large->total_weight);

    std::mt19937 random(743079);
    for (int trial = 0; trial < 390; ++trial)
    {
        const int n = static_cast<int>(random() % (trial < 210 ? 4 : 9)) + 1;
        const bool directed = trial % 2 == 0;
        std::vector<Edge> initial;
        for (int u = 0; u < n; ++u)
        {
            for (int v = directed ? 0 : u + 1; v < n; ++v)
            {
                if (u != v && random() % 5 != 0)
                {
                    initial.push_back({u, v, random() % 30 + 1});
                }
            }
        }
        check_reference(closure(n, initial, directed), directed, trial < 210);
    }
}
