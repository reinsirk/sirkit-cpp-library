// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Shortest_Weighted_Cycle.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::WeightedCycleEdge;
constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();

void enumerate(const std::vector<Edge> &edges, int v, int root, bool directed, unsigned vertices,
               unsigned used_edges, std::uint64_t distance, std::optional<std::uint64_t> &best)
{
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[a, b, cost] = edges[id];
        if ((used_edges & (1U << id)) != 0 || cost > kMax - distance)
        {
            continue;
        }
        const int to = a == v ? b : !directed && b == v ? a
                                                        : -1;
        if (to == root)
        {
            const auto value = distance + cost;
            best = best ? std::min(*best, value) : value;
        }
        else if (to != -1 && (vertices & (1U << to)) == 0)
        {
            enumerate(edges, to, root, directed, vertices | (1U << to), used_edges | (1U << id), distance + cost, best);
        }
    }
}

void check(int n, const std::vector<Edge> &edges, bool directed, std::optional<int> through)
{
    std::optional<std::uint64_t> expected;
    for (int v = through.value_or(0); v < (through ? *through + 1 : n); ++v)
    {
        enumerate(edges, v, v, directed, 1U << v, 0, 0, expected);
    }
    const auto result = sirkit::shortest_weighted_cycle(n, edges, directed, through);
    assert(result.has_value() == expected.has_value());
    if (!result)
    {
        return;
    }
    assert(result->weight == *expected);
    const auto &vertices = result->cycle.vertices;
    const auto &ids = result->cycle.edge_ids;
    assert(!vertices.empty() && vertices.size() == ids.size());
    if (through)
    {
        assert(vertices.front() == *through);
    }
    std::uint64_t sum = 0;
    std::vector<bool> used_edge(edges.size());
    std::vector<bool> seen(n);
    for (std::size_t i = 0; i < ids.size(); ++i)
    {
        assert(ids[i] < edges.size() && !used_edge[ids[i]]);
        used_edge[ids[i]] = true;
        const int u = vertices[i];
        const int v = vertices[(i + 1) % vertices.size()];
        assert(0 <= u && u < n && !seen[u]);
        seen[u] = true;
        const auto &[a, b, cost] = edges[ids[i]];
        assert((a == u && b == v) || (!directed && a == v && b == u));
        assert(cost <= kMax - sum);
        sum += cost;
    }
    assert(sum == result->weight);
}

template <class Exception, class Operation>
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

void check_limits()
{
    const std::array triangle{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 0}};
    const auto unweighted = sirkit::find_cycle(3, triangle);
    assert(unweighted && unweighted->vertices.size() == 3 && unweighted->edge_ids.size() == 3);
    check(0, {}, true, {});
    check(1, {}, true, 0);
    check(1, {{0, 0, 0}}, true, {});
    check(1, {{0, 0, kMax}}, false, {});
    check(2, {{0, 1, 1}, {1, 0, kMax - 1}}, true, 0);
    check(2, {{0, 1, 1}, {1, 0, kMax}}, true, 0);
    check(2, {{0, 1, 1}, {0, 1, 1}}, false, 0);
    check(3, {{0, 1, kMax / 2 + 4}, {0, 2, kMax / 2 + 4}, {1, 2, 2}}, false, 0);
    check(3, {{0, 1, 2}, {0, 2, 2}, {1, 2, 2}}, false, 0);
    check(3, {{0, 1, 10}, {0, 2, 1}, {2, 1, 2}}, true, 0);
    check(5, {{0, 1, 2}, {1, 2, 4}, {2, 1, 1}, {2, 4, 0}, {4, 2, 0}}, true, {});
    must_throw<std::length_error>([]
                                  {
                                      static_cast<void>(sirkit::shortest_weighted_cycle(-1, {}));
                                  });
#ifndef SKIP_RUNTIME_CHECKS
    for (int invalid : {-1, 2})
    {
        const std::vector<Edge> first{{invalid, 0, 1}};
        const std::vector<Edge> second{{0, invalid, 1}};
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(sirkit::shortest_weighted_cycle(2, first));
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(sirkit::shortest_weighted_cycle(2, second));
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(sirkit::shortest_weighted_cycle(2, {}, true, invalid));
                                      });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(1320);
    for (int t = 0; t < 850; ++t)
    {
        const int n = 1 + static_cast<int>(rng() % 7);
        const int m = static_cast<int>(rng() % 17);
        std::vector<Edge> edges;
        for (int id = 0; id < m; ++id)
        {
            const int a = static_cast<int>(rng() % n);
            const int b = static_cast<int>(rng() % n);
            edges.push_back({a, b, rng() % 11});
        }
        for (bool directed : {false, true})
        {
            check(n, edges, directed, {});
            for (int start = 0; start < n; ++start)
            {
                check(n, edges, directed, start);
            }
        }
    }
}
} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        static_cast<void>(sirkit::shortest_weighted_cycle(-1, {}));
        return 1;
    }
    check_limits();
    check_random();
}
