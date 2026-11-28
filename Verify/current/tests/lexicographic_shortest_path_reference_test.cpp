// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Lexicographic_Shortest_Path.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace
{
using Edge = sirkit::ShortestPathEdge;
using Path = std::tuple<std::uint64_t, std::vector<int>, std::vector<std::size_t>>;

void check(int n, const std::vector<Edge> &edges, int source, int target, bool undirected)
{
    std::vector<bool> seen(n);
    std::vector<int> vertices{source};
    std::vector<std::size_t> ids;
    std::optional<Path> best;
    auto visit = [&](auto &self, int u, std::uint64_t total) -> void
    {
        if (u == target)
        {
            const auto candidate = Path{total, vertices, ids};
            if (!best || candidate < *best)
            {
                best = candidate;
            }
            return;
        }
        seen[u] = true;
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const auto [a, b, weight] = edges[i];
            int v = -1;
            if (a == u)
            {
                v = b;
            }
            else if (undirected && b == u)
            {
                v = a;
            }
            if (v != -1 && !seen[v] && weight <= std::numeric_limits<std::uint64_t>::max() - total)
            {
                vertices.push_back(v);
                ids.push_back(i);
                self(self, v, total + weight);
                ids.pop_back();
                vertices.pop_back();
            }
        }
        seen[u] = false;
    };
    visit(visit, source, 0);

    const auto actual = sirkit::lexicographic_shortest_path(n, edges, source, target, undirected);
    assert(actual.has_value() == best.has_value());
    if (actual)
    {
        assert((Path{actual->distance, actual->vertices, actual->edges} == *best));
    }
}

template <typename Error, typename Action>
void check_error(Action action)
{
    bool found = false;
    try
    {
        action();
    }
    catch (const Error &)
    {
        found = true;
    }
    assert(found);
}
} // namespace

int main()
{
    check(1, {}, 0, 0, false);
    check(2, {}, 0, 1, false);
    check(5, {{0, 2, 1}, {2, 3, 1}, {0, 1, 1}, {1, 3, 1}, {0, 1, 1}, {0, 3, 2}, {0, 4, 1},
              {0, 0, 3}, {4, 3, 9}, {1, 1, 2}},
          0, 3, false);
    check(4, {{0, 1, std::numeric_limits<std::uint64_t>::max()}, {0, 2, 1}, {2, 1, 1},
              {2, 3, std::numeric_limits<std::uint64_t>::max()}},
          0, 3, true);
    check(2, {{1, 0, std::numeric_limits<std::uint64_t>::max()}}, 0, 1, true);
    std::mt19937 random(53097);
    for (int test = 0; test < 1100; ++test)
    {
        const int n = static_cast<int>(random() % 7) + 1;
        const int m = static_cast<int>(random() % 17);
        const int source = static_cast<int>(random() % n);
        const int target = static_cast<int>(random() % n);
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            const int u = static_cast<int>(random() % n);
            const int v = static_cast<int>(random() % n);
            edges.push_back({u, v, (random() % 8) + 1});
        }
        check(n, edges, source, target, test % 2 == 0);
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[source, target] : std::vector<std::pair<int, int>>{{-1, 0}, {2, 0}, {0, -1}, {0, 2}})
    {
        check_error<std::out_of_range>([&] { (void)sirkit::lexicographic_shortest_path(2, {}, source, target); });
    }
    for (const Edge edge : std::vector<Edge>{{-1, 0, 1}, {2, 0, 1}, {0, -1, 1}, {0, 2, 1}})
    {
        check_error<std::out_of_range>(
            [&] { (void)sirkit::lexicographic_shortest_path(2, std::span(&edge, 1), 0, 1); });
    }
    const std::vector<Edge> zero{{0, 1, 0}};
    check_error<std::invalid_argument>([&] { (void)sirkit::lexicographic_shortest_path(2, zero, 0, 1); });
#endif
}
