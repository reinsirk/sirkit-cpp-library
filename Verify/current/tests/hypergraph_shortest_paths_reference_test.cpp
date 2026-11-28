// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Hypergraph_Shortest_Paths.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Weight = std::uint64_t;
using Distance = std::optional<Weight>;
using Arc = std::pair<int, Weight>;
using Group = std::pair<std::vector<int>, Weight>;
using Edges = std::vector<std::vector<Arc>>;
constexpr auto kMax = std::numeric_limits<Weight>::max();

void relax(Distance &distance, Weight candidate)
{
    if (!distance || candidate < *distance)
    {
        distance = candidate;
    }
}

std::vector<sirkit::Hyperedge> spans(const std::vector<Group> &groups)
{
    std::vector<sirkit::Hyperedge> result;
    for (const auto &[members, cost] : groups)
    {
        result.push_back({members, cost});
    }
    return result;
}

void check(int n, const std::vector<Group> &groups, const Edges &adj, bool use_unit)
{
    auto index = [&]
    {
        const auto group_edges = spans(groups);
        return sirkit::HypergraphShortestPaths(n, group_edges);
    }();
    std::vector distance(n, std::vector<Distance>(n));
    for (int v = 0; v < n; ++v)
    {
        distance[v][v] = 0;
        for (const auto &[u, weight] : adj[v])
        {
            relax(distance[v][u], weight);
        }
    }
    for (const auto &[members, cost] : groups)
    {
        for (const int v : members)
        {
            for (const int u : members)
            {
                relax(distance[v][u], cost);
            }
        }
    }
    for (int k = 0; k < n; ++k)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                if (distance[u][k] && distance[k][v] && *distance[u][k] <= kMax - *distance[k][v])
                {
                    relax(distance[u][v], *distance[u][k] + *distance[k][v]);
                }
            }
        }
    }
    auto weighted = [&](int v, auto emit)
    {
        for (const auto &[u, cost] : adj[v])
        {
            emit(u, cost);
        }
    };
    auto unit = [&](int v, auto emit)
    {
        for (const auto &[u, cost] : adj[v])
        {
            emit(u);
        }
    };
    for (int v = 0; v < n; ++v)
    {
        assert(index.weighted_distances(v, weighted) == distance[v]);
        if (use_unit)
        {
            const auto answer = index.unit_distances(v, unit);
            for (int u = 0; u < n; ++u)
            {
                assert(answer[u] == (distance[v][u] ? static_cast<int>(*distance[v][u]) : -1));
            }
        }
    }
}

template <typename Error, typename Action>
void check_error(Action action)
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
    check(0, {}, {}, true);
    check(1, {{{}, 1}, {{0, 0}, 1}}, Edges(1), true);
    check(3, {{{0, 1, 1}, 1}, {{1, 2}, 1}}, Edges(3), true);
    check(4, {{{0, 1}, kMax}, {{1, 2}, 0}, {{1, 3}, 1}}, Edges(4), false);
    check(5, {{{0, 3}, 16}, {{0, 1}, 1}, {{1, 3}, 3}, {{3, 4}, 20}},
          Edges{{{4, 90}}, {}, {}, {}, {}}, false);
    check(5, {{{1, 2}, 1}, {{2, 3}, 1}, {{3, 4}, 1}},
          Edges{{{1, 1}, {4, 1}}, {}, {}, {}, {{2, 1}}}, true);

    std::mt19937 random(302184);
    for (int test = 0; test < 920; ++test)
    {
        const int n = 1 + static_cast<int>(random() % 8);
        Edges adj(n);
        const bool weighted = random() % 2 != 0;
        auto cost = [&] { return weighted ? Weight(random() % 15) : 1; };
        for (int v = 0; v < n; ++v)
        {
            for (int u = 0; u < n; ++u)
            {
                if (random() % 3 == 0)
                {
                    adj[v].emplace_back(u, cost());
                }
            }
        }
        std::vector<Group> groups(random() % 8);
        for (auto &[members, weight] : groups)
        {
            members.resize(random() % 7);
            weight = cost();
            for (int &v : members)
            {
                v = static_cast<int>(random() % n);
            }
        }
        check(n, groups, adj, !weighted);
    }

    const sirkit::HypergraphShortestPaths unit(3, {});
    assert((unit.unit_distances(1) == std::vector<int>{-1, 0, -1}));
    assert((unit.weighted_distances(1) == std::vector<Distance>{std::nullopt, 0, std::nullopt}));
    const std::vector<Group> with_weights{{{0, 1}, 3}, {{1, 2}, 0}, {{0, 2}, 8}};
    const auto group_edges = spans(with_weights);
    const sirkit::HypergraphShortestPaths heavy(3, group_edges);
    assert((heavy.weighted_distances(0) == std::vector<Distance>{0, 3, 3}));
    check_error<std::invalid_argument>([&] { (void)heavy.unit_distances(0); });
    check_error<std::length_error>([] { sirkit::HypergraphShortestPaths invalid(-1, {}); });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int invalid : {-1, 3})
    {
        const sirkit::Hyperedge bad{std::span(&invalid, 1)};
        check_error<std::out_of_range>([&] { sirkit::HypergraphShortestPaths graph(3, std::span(&bad, 1)); });
        check_error<std::out_of_range>([&] { (void)unit.unit_distances(invalid); });
        check_error<std::out_of_range>([&] { (void)unit.weighted_distances(invalid); });
        check_error<std::out_of_range>([&] { (void)unit.unit_distances(0, [&](int, auto emit) { emit(invalid); }); });
        check_error<std::out_of_range>(
            [&] { (void)unit.weighted_distances(0, [&](int, auto emit) { emit(invalid, 2); }); });
    }
#endif
}
