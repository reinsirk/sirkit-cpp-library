// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Counting_Spanning_Trees.cpp"
#include "reference_modint.hpp"

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Mint = sirkit_verify::ReferenceModInt<>;
using Edge = sirkit::SpanningTreeCountEdge<Mint>;

struct ReferenceEdge
{
    int from;
    int to;
    std::int64_t weight;
};

std::int64_t reference(int n, int root, const std::vector<ReferenceEdge> &edges, bool directed)
{
    std::int64_t total = 0;
    const auto limit = 1U << edges.size();
    for (unsigned subset = 0; subset < limit; ++subset)
    {
        if (std::popcount(subset) != n - 1)
        {
            continue;
        }
        std::array<unsigned, 8> graph{};
        std::int64_t weight = 1;
        for (auto left = subset; left != 0; left &= left - 1)
        {
            const auto &[from, to, value] = edges[std::countr_zero(left)];
            graph[from] |= 1U << to;
            if (!directed)
            {
                graph[to] |= 1U << from;
            }
            weight *= value;
        }

        unsigned seen = 1U << root;
        unsigned frontier = seen;
        while (frontier != 0)
        {
            const auto next = graph[std::countr_zero(frontier)] & ~seen;
            frontier = (frontier & (frontier - 1)) | next;
            seen |= next;
        }
        if (seen == (1U << n) - 1)
        {
            total += weight;
        }
    }
    return total;
}

void check(int n, int root, const std::vector<ReferenceEdge> &raw)
{
    std::vector<Edge> edges;
    for (const auto &[from, to, weight] : raw)
    {
        edges.push_back({from, to, weight});
    }
    const auto undirected = sirkit::count_undirected_spanning_trees<Mint>(n, edges);
    const auto directed = sirkit::count_directed_spanning_trees<Mint>(n, root, edges);
    assert(undirected == Mint{reference(n, root, raw, false)});
    assert(directed == Mint{reference(n, root, raw, true)});
}

template <class Exception>
void expect_error(const auto &operation)
{
    try
    {
        operation();
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
} // namespace

int main()
{
    Mint::set_mod(5);
    check(1, 0, {});
    check(1, 0, {{0, 0, 7}, {0, 0, -5}});
    check(3, 0, {{0, 1, -1}, {2, 1, 1}, {1, 2, 1}});
    check(3, 0, {{1, 0, 4}, {2, 1, 1}});
    check(3, 2, {{1, 0, 4}, {2, 1, 1}});
    check(3, 2, {{1, 0, 0}, {2, 1, 1}});
    check(3, 1, {{1, 0, 4}, {1, 0, 1}, {2, 1, 1}});

    std::mt19937 random(81502);
    for (const int mod : {2, 17, 998244353})
    {
        Mint::set_mod(mod);
        for (int n = 1; n <= 8; ++n)
        {
            for (int trial = 0; trial < 100; ++trial)
            {
                const int root = static_cast<int>(random() % n);
                const auto m = random() % 12;
                std::vector<ReferenceEdge> edges;
                for (std::size_t i = 0; i < m; ++i)
                {
                    edges.push_back({static_cast<int>(random() % n), static_cast<int>(random() % n),
                                     static_cast<std::int64_t>(random() % 19) - 9});
                }
                check(n, root, edges);
            }
        }
    }

    expect_error<std::invalid_argument>([]
                                        {
                                            static_cast<void>(sirkit::count_undirected_spanning_trees<Mint>(0, {}));
                                        });
    expect_error<std::length_error>([]
                                    {
                                        static_cast<void>(sirkit::count_undirected_spanning_trees<Mint>(std::numeric_limits<int>::max(), {}));
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    for (int root : {-1, 3})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            static_cast<void>(sirkit::count_directed_spanning_trees<Mint>(3, root, {}));
                                        });
    }
    for (const auto &edge : {Edge{-1, 0}, Edge{3, 0}, Edge{0, -1}, Edge{0, 3}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            static_cast<void>(sirkit::count_undirected_spanning_trees<Mint>(3, {&edge, 1}));
                                        });
    }
#endif

    constexpr int kSize = 64;
    std::vector<Edge> undirected;
    std::vector<Edge> directed;
    for (int v = 0; v < kSize; ++v)
    {
        for (int u = 0; u < v; ++u)
        {
            undirected.push_back({u, v});
            directed.push_back({u, v});
            directed.push_back({v, u});
        }
    }
    const auto cayley = Mint{kSize}.pow(kSize - 2);
    assert(sirkit::count_undirected_spanning_trees<Mint>(kSize, undirected) == cayley);
    assert(sirkit::count_directed_spanning_trees<Mint>(kSize, 29, directed) == cayley);

    using EulerEdge = std::pair<int, int>;
    const auto count_euler = [](int n, const std::vector<EulerEdge> &edges)
    {
        return sirkit::count_eulerian_circuits<Mint>(n, edges);
    };
    assert(count_euler(0, {}) == Mint{1});
    assert(count_euler(3, {{1, 1}, {1, 1}, {1, 1}, {1, 1}}) == Mint{6});
    assert(count_euler(2, {{0, 0}, {1, 1}}) == Mint{0});
    for (int trial = 0; trial < 850; ++trial)
    {
        const int n = static_cast<int>(random() % 5) + 1;
        const std::size_t m = random() % 8 + 1;
        std::vector<EulerEdge> edges(m);
        for (auto &[u, v] : edges)
        {
            u = static_cast<int>(random() % n);
            v = static_cast<int>(random() % n);
        }
        std::vector<std::size_t> order(m);
        std::iota(order.begin(), order.end(), std::size_t{0});
        unsigned count = 0;
        do
        {
            bool circuit = true;
            for (std::size_t j = 0; j < m; ++j)
            {
                circuit &= edges[order[j]].second == edges[order[(j + 1) % m]].first;
            }
            count += circuit;
        } while (std::next_permutation(order.begin() + 1, order.end()));
        assert(count_euler(n, edges) == Mint{count});
    }
    expect_error<std::invalid_argument>([&]
                                        {
                                            count_euler(-1, {});
                                        });
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &invalid : {EulerEdge{-1, 0}, EulerEdge{2, 0}, EulerEdge{0, -1}, EulerEdge{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            count_euler(2, {invalid});
                                        });
    }
#endif
}
