// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Range_Edge_Graph.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Weight = std::int64_t;
constexpr Weight kInfinity = std::numeric_limits<Weight>::max();
using Matrix = std::vector<std::vector<Weight>>;

template<class Exception, class Operation>
void must_throw(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

std::vector<Weight> distances(const sirkit::WGraph &graph, int start)
{
    std::vector<Weight> result(graph.size(), kInfinity);
    std::priority_queue<std::pair<Weight, int>, std::vector<std::pair<Weight, int>>, std::greater<>> queue;
    result[start] = 0;
    queue.emplace(0, start);
    while (!queue.empty())
    {
        const auto [value, v] = queue.top();
        queue.pop();
        if (value != result[v])
        {
            continue;
        }
        for (const auto &edge : graph[v])
        {
            assert(edge.w >= 0);
            if (value + edge.w < result[edge.to])
            {
                result[edge.to] = value + edge.w;
                queue.emplace(result[edge.to], edge.to);
            }
        }
    }
    return result;
}

void check_exceptions()
{
    sirkit::RangeEdgeGraph empty(0);
    assert(empty.size() == 0 && empty.vertex_count() == 0 && empty.graph().empty());
    empty.add_range_edge(0, 0, 0, 0, 1);
    assert(empty.add_layer({}, {}) == 1);
    must_throw<std::length_error>([] { const sirkit::RangeEdgeGraph graph(-1); });
    must_throw<std::length_error>([] { sirkit::detail::check_range_edge_vertex_count(std::numeric_limits<int>::max(), 1); });
    const std::array<Weight, 2> zero{}, positive{1, 1}, negative{-1, -1};
    const std::array<Weight, 2> wide{std::numeric_limits<Weight>::min(), std::numeric_limits<Weight>::max()};
    sirkit::RangeEdgeGraph graph(2);
    must_throw<std::overflow_error>([&] { graph.add_layer(wide, zero); });
    assert(graph.vertex_count() == 4 && graph.edges().size() == 4);
#ifndef SKIP_RUNTIME_CHECKS
    must_throw<std::invalid_argument>([&] { graph.add_layer({}, zero); });
    must_throw<std::invalid_argument>([&] { graph.add_layer(zero, {}); });
    must_throw<std::out_of_range>([&] { graph.add_edge(-1, 0, 0); });
    must_throw<std::out_of_range>([&] { graph.add_edge(0, 2, 0); });
    must_throw<std::out_of_range>([&] { graph.add_range_edge(-1, 1, 0, 2, 0); });
    must_throw<std::out_of_range>([&] { graph.add_range_edge(1, 0, 0, 2, 0); });
    must_throw<std::out_of_range>([&] { graph.add_range_edge(0, 3, 0, 2, 0); });
    must_throw<std::out_of_range>([&] { graph.add_range_edge(0, 2, 0, 2, 0, 1); });
#endif
    const auto plus = graph.add_layer(positive, zero);
    const auto minus = graph.add_layer(negative, zero);
    assert(plus == 1 && minus == 2);
    must_throw<std::overflow_error>([&] { graph.add_range_edge(0, 1, 1, 2, std::numeric_limits<Weight>::max(), plus); });
    must_throw<std::overflow_error>([&] { graph.add_range_edge(0, 1, 1, 2, std::numeric_limits<Weight>::min(), minus); });
    assert(graph.vertex_count() == 8 && graph.edges().size() == 12);

    // 負の2項を先に足すと中間だけがあふれる。異符号の項は先に相殺できる。
    const std::array<Weight, 2> minimum{std::numeric_limits<Weight>::min(), std::numeric_limits<Weight>::min()};
    const auto cancel = graph.add_layer(negative, minimum);
    graph.add_range_edge(0, 1, 1, 2, std::numeric_limits<Weight>::max(), cancel);
    assert(std::get<2>(graph.edges()[graph.edges().size() - 2]) == -2);
    const auto positive_far = graph.add_layer(positive, minimum);
    graph.add_range_edge(0, 2, 0, 2, std::numeric_limits<Weight>::max(), positive_far);
    assert(std::get<2>(graph.edges()[graph.edges().size() - 2]) == 0);
}

void compare_directed_graphs()
{
    std::mt19937 random(20261128);
    for (int n = 1; n <= 24; ++n)
    {
        for (int trial = 0; trial < 8; ++trial)
        {
            sirkit::RangeEdgeGraph compressed(n);
            Matrix plain(n, std::vector<Weight>(n, kInfinity));
            std::vector<Weight> from(n), to(n);
            for (int v = 0; v < n; ++v)
            {
                from[v] = static_cast<Weight>(random() % 21) - 10;
                to[v] = static_cast<Weight>(random() % 21) - 10;
                plain[v][v] = 0;
            }
            const auto layer = compressed.add_layer(from, to);
            const auto zero = std::vector<Weight>(n);
            assert(compressed.add_layer(zero, zero) == layer + 1);
            for (int q = 0; q < 60; ++q)
            {
                const int i = static_cast<int>(random() % n);
                const int j = static_cast<int>(random() % n);
                if (q % 3 == 0)
                {
                    const Weight cost = random() % 80;
                    compressed.add_edge(i, j, cost);
                    plain[i][j] = std::min(plain[i][j], cost);
                }
                else
                {
                    const int i2 = static_cast<int>(random() % (n + 1));
                    const int j2 = static_cast<int>(random() % (n + 1));
                    const auto [a, b] = std::minmax(i, i2);
                    const auto [c, d] = std::minmax(j, j2);
                    const Weight base = 20 + random() % 80;
                    const bool use_layer = q % 3 == 1;
                    compressed.add_range_edge(a, b, c, d, base, use_layer ? layer : 0);
                    for (int u = a; u < b; ++u)
                    {
                        for (int v = c; v < d; ++v)
                        {
                            const Weight weight = base + (use_layer ? from[u] + to[v] : 0);
                            plain[u][v] = std::min(plain[u][v], weight);
                        }
                    }
                }
            }
            const auto topology = compressed.graph();
            assert(static_cast<int>(topology.size()) == compressed.vertex_count() && compressed.size() == n);
            for (int k = 0; k < n; ++k)
            {
                for (int i = 0; i < n; ++i)
                {
                    for (int j = 0; j < n; ++j)
                    {
                        if (plain[i][k] != kInfinity && plain[k][j] != kInfinity)
                        {
                            plain[i][j] = std::min(plain[i][j], plain[i][k] + plain[k][j]);
                        }
                    }
                }
            }
            for (int start = 0; start < n; ++start)
            {
                assert(std::equal(plain[start].begin(), plain[start].end(), distances(topology, start).begin()));
            }
        }
    }
}
}

int main()
{
    check_exceptions();
    compare_directed_graphs();
}
