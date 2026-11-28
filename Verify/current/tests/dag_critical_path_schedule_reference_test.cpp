// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/DAG_Critical_Path_Schedule.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Edge = sirkit::DagScheduleEdge;
using Time = std::optional<std::uint64_t>;

void check(int n, const std::vector<Edge> &edges, int start, int finish)
{
    std::vector reach(n, std::vector<bool>(n));
    for (const auto &[u, v, w] : edges)
    {
        reach[u][v] = true;
    }
    for (int k = 0; k < n; ++k)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                reach[u][v] = reach[u][v] || (reach[u][k] && reach[k][v]);
            }
        }
    }
    const auto actual = sirkit::dag_critical_path_schedule(n, edges, start, finish);
    for (int u = 0; u < n; ++u)
    {
        if (reach[u][u])
        {
            assert(!actual);
            return;
        }
    }

    std::vector<Time> prefix(n), suffix(n), through(edges.size());
    std::vector<int> vertices{start};
    std::vector<std::uint64_t> time{0};
    std::vector<std::size_t> used_edges;
    Time longest;
    auto add = [](Time &destination, std::uint64_t candidate)
    {
        if (!destination || candidate > *destination)
        {
            destination = candidate;
        }
    };
    auto visit = [&](auto &self, int u) -> void
    {
        if (u == finish)
        {
            const auto duration = time.back();
            add(longest, duration);
            for (std::size_t i = 0; i < vertices.size(); ++i)
            {
                add(prefix[vertices[i]], time[i]);
                add(suffix[vertices[i]], duration - time[i]);
            }
            for (const auto id : used_edges)
            {
                add(through[id], duration);
            }
            return;
        }
        for (std::size_t id = 0; id < edges.size(); ++id)
        {
            const auto &[from, to, duration] = edges[id];
            if (from == u)
            {
                used_edges.push_back(id);
                vertices.push_back(to);
                time.push_back(time.back() + duration);
                self(self, to);
                time.pop_back();
                vertices.pop_back();
                used_edges.pop_back();
            }
        }
    };
    visit(visit, start);
    assert(actual.has_value() == longest.has_value());
    if (!actual)
    {
        return;
    }
    assert(actual->duration == longest);
    assert(actual->earliest == prefix);
    for (int u = 0; u < n; ++u)
    {
        assert(actual->latest[u] == (suffix[u] ? Time{*longest - *suffix[u]} : std::nullopt));
    }
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        assert(actual->edge_slack[id] == (through[id] ? Time{*longest - *through[id]} : std::nullopt));
    }
    int u = start;
    std::uint64_t duration = 0;
    for (const auto id : actual->critical_path)
    {
        assert(id < edges.size());
        const auto edge = edges[id];
        assert(edge.from == u);
        assert(actual->edge_slack[id] == 0);
        u = edge.to;
        duration += edge.duration;
    }
    assert(u == finish && duration == longest);
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
    check(1, {}, 0, 0);
    check(3, {}, 0, 2);
    check(4, {{0, 1, 1}, {2, 3, 3}, {3, 2, 2}}, 0, 1);
    check(7, {{0, 1, 2}, {0, 1, 1}, {0, 2, 1}, {0, 3, 20}, {1, 4, 1}, {2, 4, 2}, {3, 6, 30},
              {4, 5, 0}, {6, 3, 2}},
          0, 5);
    check(7, {{0, 1, 2}, {0, 1, 1}, {0, 2, 1}, {0, 3, 20}, {1, 4, 1}, {2, 4, 2}, {3, 6, 30},
              {4, 5, 0}, {2, 5, 0}},
          0, 5);
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    check(3, {{0, 1, kMax}, {1, 2, 0}, {0, 2, 1}}, 0, 2);
    const std::vector<Edge> irrelevant_overflow{{0, 1, kMax}, {1, 2, 10}, {0, 3, 1}};
    const auto irrelevant = sirkit::dag_critical_path_schedule(4, irrelevant_overflow, 0, 3);
    assert(irrelevant && irrelevant->duration == 1);
    assert(!irrelevant->earliest[1] && !irrelevant->edge_slack[0]);
    const std::vector<Edge> overflow{{0, 1, kMax}, {1, 2, 1}};
    check_error<std::overflow_error>([&] { (void)sirkit::dag_critical_path_schedule(3, overflow, 0, 2); });

    std::mt19937 random(7600913);
    for (int test = 0; test < 780; ++test)
    {
        const int n = static_cast<int>(random() % 9) + 1;
        std::vector<int> perm(n);
        std::iota(perm.begin(), perm.end(), 0);
        std::shuffle(perm.begin(), perm.end(), random);
        std::vector<Edge> edges;
        for (int u = 0; u < n; ++u)
        {
            for (int v = u + 1; v < n; ++v)
            {
                if (random() % 3 == 0)
                {
                    edges.push_back({perm[u], perm[v], random() % 9});
                }
            }
        }
        if (test % 7 == 0)
        {
            edges.push_back({perm[0], perm[0], 0});
        }
        check(n, edges, static_cast<int>(random() % n), static_cast<int>(random() % n));
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[s, t] : std::vector<std::pair<int, int>>{{-1, 0}, {2, 0}, {0, -1}, {0, 2}})
    {
        check_error<std::out_of_range>([&] { (void)sirkit::dag_critical_path_schedule(2, {}, s, t); });
    }
    for (const Edge edge : std::vector<Edge>{{-1, 0, 1}, {2, 0, 1}, {0, -1, 1}, {0, 2, 1}})
    {
        check_error<std::out_of_range>(
            [&] { (void)sirkit::dag_critical_path_schedule(2, std::span(&edge, 1), 0, 1); });
    }
#endif
}
