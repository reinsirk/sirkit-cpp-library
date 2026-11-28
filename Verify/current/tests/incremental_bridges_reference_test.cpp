// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Incremental_Bridges.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

void check(int n, const std::vector<Edge> &edges)
{
    sirkit::IncrementalBridges actual(n);
    actual.reserve_edges(edges.size());
    assert(actual.size() == n);
    assert(actual.edge_count() == 0);
    assert(actual.bridge_count() == 0);
    assert(actual.component_count() == n);
    assert(actual.block_count() == n);
    std::vector<std::vector<std::pair<int, std::size_t>>> graph(n);
    constexpr auto kNoSkip = std::numeric_limits<std::size_t>::max();
    auto traverse = [&](int source, std::size_t omit, const std::vector<bool> &blocked)
    {
        std::vector<bool> seen(n);
        seen[source] = true;
        std::vector<int> queue{source};
        for (std::size_t i = 0; i < queue.size(); ++i)
        {
            for (const auto &[v, id] : graph[queue[i]])
            {
                if (id != omit && (blocked.empty() || !blocked[id]) && !seen[v])
                {
                    seen[v] = true;
                    queue.push_back(v);
                }
            }
        }
        return seen;
    };
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v] = edges[id];
        const auto inserted = actual.add_edge(u, v);
        assert(inserted == id);
        assert(actual.edge_count() == id + 1);
        graph[u].emplace_back(v, id);
        graph[v].emplace_back(u, id);

        std::vector<bool> bridges(id + 1);
        for (std::size_t j = 0; j <= id; ++j)
        {
            const auto [from, to] = edges[j];
            bridges[j] = !traverse(from, j, {})[to];
            assert(actual.is_bridge(j) == bridges[j]);
        }
        assert(actual.bridge_count() == std::ranges::count(bridges, true));

        int components = 0;
        int blocks = 0;
        for (int from = 0; from < n; ++from)
        {
            const auto connected = traverse(from, kNoSkip, {});
            const auto block = traverse(from, kNoSkip, bridges);
            components += std::none_of(connected.begin(), connected.begin() + from, [](bool value) { return value; });
            blocks += std::none_of(block.begin(), block.begin() + from, [](bool value) { return value; });
            for (int to = 0; to < n; ++to)
            {
                assert(actual.same_component(from, to) == connected[to]);
                assert(actual.same_two_edge_component(from, to) == block[to]);
            }
        }
        assert(actual.component_count() == components);
        assert(actual.block_count() == blocks);
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
    check(0, {});
    check(1, {{0, 0}, {0, 0}});
    check(5, {{0, 1}, {0, 1}, {1, 2}, {1, 3}, {2, 3}, {3, 4}, {3, 4}});
    check(8, {{0, 1}, {1, 2}, {2, 3}, {4, 5}, {5, 6}, {3, 4}, {1, 5}, {6, 7}, {0, 7}});
    std::mt19937 random(831832);
    for (int test = 0; test < 350; ++test)
    {
        const int n = static_cast<int>(random() % 9) + 1;
        const int m = static_cast<int>(random() % 25);
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            const int u = static_cast<int>(random() % n);
            const int v = static_cast<int>(random() % n);
            edges.emplace_back(u, v);
        }
        check(n, edges);
    }

    const int n = 1 << 15;
    sirkit::IncrementalBridges chain(n);
    chain.reserve_edges(n);
    for (int step = 1; step < n; step *= 2)
    {
        for (int v = 0; v < n; v += 2 * step)
        {
            chain.add_edge(v + step - 1, v + step);
        }
        assert(chain.bridge_count() == n - n / (2 * step));
    }
    auto copy = chain;
    copy.add_edge(0, n - 1);
    assert(copy.bridge_count() == 0 && copy.block_count() == 1);
    assert(chain.bridge_count() == n - 1);
    assert(chain.is_bridge(n - 2));

#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::length_error>(
        [] { (void)sirkit::IncrementalBridges(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1); });
    sirkit::IncrementalBridges tiny(2);
    check_error<std::out_of_range>([&] { (void)tiny.is_bridge(0); });
    check_error<std::out_of_range>([&] { (void)tiny.add_edge(-1, 0); });
    check_error<std::out_of_range>([&] { (void)tiny.add_edge(0, 2); });
    assert(tiny.edge_count() == 0);
#endif
}
