// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Vertex_Deletion_Components.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

std::pair<int, std::vector<int>> label_components(const std::vector<std::vector<int>> &graph, int deleted)
{
    const int size = static_cast<int>(graph.size());
    std::vector<int> components(size, -1);
    int count = 0;
    for (int start = 0; start < size; ++start)
    {
        if (start == deleted || components[start] != -1)
        {
            continue;
        }
        std::vector<int> next{start};
        components[start] = count;
        while (!next.empty())
        {
            const int v = next.back();
            next.pop_back();
            for (const int w : graph[v])
            {
                if (w != deleted && components[w] == -1)
                {
                    components[w] = count;
                    next.push_back(w);
                }
            }
        }
        ++count;
    }
    return {count, components};
}

void compare(int n, const std::vector<Edge> &edges)
{
    const auto answer = sirkit::vertex_deletion_components(n, edges);
    std::vector<std::vector<int>> graph(n);
    for (const auto &[u, v] : edges)
    {
        graph[u].push_back(v);
        graph[v].push_back(u);
    }
    const auto [before, old_component] = label_components(graph, -1);
    assert(answer.original_count == before);
    assert(answer.after_deleting.size() == static_cast<std::size_t>(n));
    assert(answer.separated_pairs.size() == static_cast<std::size_t>(n));
    for (int removed = 0; removed < n; ++removed)
    {
        const auto [remaining, new_component] = label_components(graph, removed);
        assert(answer.after_deleting[removed] == remaining);
        std::uint64_t pairs = 0;
        for (int u = 0; u < n; ++u)
        {
            for (int v = u + 1; v < n; ++v)
            {
                if (u != removed && v != removed && old_component[u] == old_component[v] && new_component[u] != new_component[v])
                {
                    ++pairs;
                }
            }
        }
        assert(answer.separated_pairs[removed] == pairs);
    }
}

template <class Error, class Action>
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
    compare(0, {});
    compare(1, {});
    compare(1, {{0, 0}, {0, 0}});
    compare(7, {{0, 1}, {1, 2}, {2, 0}, {1, 3}, {3, 4}, {1, 4}, {1, 1}, {1, 3}, {5, 6}});
    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::vertex_deletion_components(-1, {});
                                    });
    for (const auto &[u, v] : {Edge{-1, 0}, Edge{3, 0}, Edge{0, -1}, Edge{0, 3}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array edges{Edge{u, v}};
                                            (void)sirkit::vertex_deletion_components(3, edges);
                                        });
    }

    constexpr int kSmall = 5;
    const auto pairs = []
    {
        std::vector<Edge> edges;
        for (int v = 0; v < kSmall; ++v)
        {
            for (int u = 0; u < v; ++u)
            {
                edges.emplace_back(u, v);
            }
        }
        return edges;
    }();
    for (unsigned mask = 0; mask < (1U << pairs.size()); ++mask)
    {
        std::vector<Edge> edges;
        for (std::size_t i = 0; i < pairs.size(); ++i)
        {
            if (mask >> i & 1U)
            {
                edges.push_back(pairs[i]);
            }
        }
        compare(kSmall, edges);
    }

    std::mt19937 random(3022026);
    for (int trial = 0; trial < 450; ++trial)
    {
        const int n = static_cast<int>(random() % 10) + 1;
        std::vector<Edge> edges;
        const auto edge_count = random() % 34;
        for (std::size_t i = 0; i < edge_count; ++i)
        {
            edges.emplace_back(random() % n, random() % n);
        }
        compare(n, edges);
    }

    constexpr int kSize = 200000;
    std::vector<Edge> chain;
    for (int v = 1; v < kSize; ++v)
    {
        chain.emplace_back(v - 1, v);
    }
    const auto answer = sirkit::vertex_deletion_components(kSize, chain);
    assert(answer.original_count == 1);
    for (int v = 0; v < kSize; ++v)
    {
        assert(answer.after_deleting[v] == (v == 0 || v == kSize - 1 ? 1 : 2));
        assert(answer.separated_pairs[v] == static_cast<std::uint64_t>(v) * (kSize - 1 - v));
    }
}
