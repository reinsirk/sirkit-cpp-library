// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Shortest_Path_Edge_Usage.cpp"

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
using Edge = sirkit::ShortestPathEdge;
using Use = sirkit::ShortestPathEdgeUse;

struct EveryMinimum
{
    std::optional<std::uint64_t> distance;
    std::vector<bool> seen;
    std::vector<bool> necessary;
};

void enumerate(const std::vector<Edge> &edges, int from, int target, bool undirected,
               unsigned visited, std::uint64_t mask, std::uint64_t weight, EveryMinimum &paths)
{
    if (from == target)
    {
        if (!paths.distance || weight < *paths.distance)
        {
            paths.distance = weight;
            for (std::size_t id = 0; id < edges.size(); ++id)
            {
                paths.seen[id] = paths.necessary[id] = ((mask >> id) & 1) != 0;
            }
        }
        else if (weight == *paths.distance)
        {
            for (std::size_t id = 0; id < edges.size(); ++id)
            {
                const bool uses_edge = ((mask >> id) & 1) != 0;
                paths.seen[id] = paths.seen[id] || uses_edge;
                paths.necessary[id] = paths.necessary[id] && uses_edge;
            }
        }
        return;
    }
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[u, v, length] = edges[id];
        int next = -1;
        if (u == from)
        {
            next = v;
        }
        else if (undirected && v == from)
        {
            next = u;
        }
        if (next != -1 && !(visited & (1U << next)))
        {
            enumerate(edges, next, target, undirected, visited | (1U << next),
                      mask | (std::uint64_t{1} << id), weight + length, paths);
        }
    }
}

void check_random()
{
    std::mt19937 rng(375);
    for (int trial = 0; trial < 1300; ++trial)
    {
        const int n = 1 + static_cast<int>(rng() % 7);
        const int m = static_cast<int>(rng() % 17);
        const int source = static_cast<int>(rng() % n);
        const int target = static_cast<int>(rng() % n);
        const bool undirected = (trial % 2) != 0;
        std::vector<Edge> edges;
        for (int id = 0; id < m; ++id)
        {
            edges.push_back({static_cast<int>(rng() % n), static_cast<int>(rng() % n), 1 + rng() % 8});
        }
        EveryMinimum paths{std::nullopt, std::vector<bool>(m), std::vector<bool>(m)};
        enumerate(edges, source, target, undirected, 1U << source, 0, 0, paths);
        const auto result = sirkit::shortest_path_edge_usage(n, edges, source, target, undirected);
        assert(result.distance == paths.distance);
        for (int id = 0; id < m; ++id)
        {
            assert(result.edge_use[id] == (paths.necessary[id] ? Use::all : paths.seen[id] ? Use::some : Use::none));
        }
    }
}

void check_boundary()
{
    constexpr auto kLimit = std::numeric_limits<std::uint64_t>::max();
    const std::array exact_limit{Edge{0, 1, kLimit - 2}, Edge{1, 3, 2}, Edge{0, 3, kLimit},
                                 Edge{1, 2, 3}, Edge{3, 0, 1}};
    const auto largest = sirkit::shortest_path_edge_usage(4, exact_limit, 0, 3);
    assert(largest.distance == kLimit);
    assert(largest.edge_use == std::vector<Use>({Use::some, Use::some, Use::some, Use::none, Use::none}));
    const std::array overflowing{Edge{0, 1, kLimit}, Edge{1, 2, 1}};
    const auto overflow = sirkit::shortest_path_edge_usage(3, overflowing, 0, 2);
    assert(!overflow.distance);
    assert(overflow.edge_use == std::vector<Use>(2, Use::none));

    const std::array parallel{Edge{2, 1, 5}, Edge{2, 1, 5}, Edge{1, 0, 6}, Edge{2, 2, 1}};
    const auto tie = sirkit::shortest_path_edge_usage(3, parallel, 0, 2, true);
    assert(tie.distance == 11);
    assert(tie.edge_use == std::vector<Use>({Use::some, Use::some, Use::all, Use::none}));
    const auto empty = sirkit::shortest_path_edge_usage(3, parallel, 1, 1);
    assert(empty.distance == 0 && empty.edge_use == std::vector<Use>(4, Use::none));
    assert(!sirkit::shortest_path_edge_usage(2, {}, 0, 1).distance);
}

#ifndef SKIP_RUNTIME_CHECKS
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

void check_invalid_inputs()
{
    for (const auto &[source, target] : {std::pair{-1, 0}, std::pair{2, 0}, std::pair{0, -1}, std::pair{0, 2}})
    {
        must_throw<std::out_of_range>([&] { static_cast<void>(sirkit::shortest_path_edge_usage(2, {}, source, target)); });
    }
    for (const Edge &edge : {Edge{-1, 0, 1}, Edge{2, 0, 1}, Edge{0, -1, 1}, Edge{0, 2, 1}})
    {
        must_throw<std::out_of_range>([&] { static_cast<void>(sirkit::shortest_path_edge_usage(2, {&edge, 1}, 0, 1)); });
    }
    const std::array edges{Edge{0, 1, 0}};
    must_throw<std::invalid_argument>([&] { static_cast<void>(sirkit::shortest_path_edge_usage(2, edges, 0, 1)); });
}
#endif
} // namespace

int main(int argc, char **argv)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        static_cast<void>(sirkit::shortest_path_edge_usage(1, {}, -1, 0));
        return 1;
    }
    check_invalid_inputs();
#endif
    static_cast<void>(argc);
    static_cast<void>(argv);
    check_boundary();
    check_random();
}
