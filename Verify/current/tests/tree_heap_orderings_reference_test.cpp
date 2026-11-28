// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Heap_Orderings.cpp"

#include <algorithm>
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

std::vector<std::uint32_t> brute(int n, const std::vector<Edge> &edges, std::uint32_t prime)
{
    const unsigned size = 1U << n;
    std::vector<unsigned> adjacency(n);
    for (const auto &[u, v] : edges)
    {
        adjacency[u] |= 1U << v;
        adjacency[v] |= 1U << u;
    }
    std::vector<std::uint32_t> answer(n);
    for (int start = 0; start < n; ++start)
    {
        std::vector<std::uint64_t> ways(size);
        ways[1U << start] = 1;
        for (unsigned state = 1; state < size; ++state)
        {
            if (ways[state] == 0)
            {
                continue;
            }
            for (int next = 0; next < n; ++next)
            {
                const unsigned bit = 1U << next;
                if ((state & bit) == 0 && (state & adjacency[next]) != 0)
                {
                    ways[state | bit] += ways[state];
                }
            }
        }
        answer[start] = static_cast<std::uint32_t>(ways.back() % prime);
    }
    return answer;
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
    assert(sirkit::tree_heap_orderings(0, {}).empty());
    assert((sirkit::tree_heap_orderings(1, {}, 2) == std::vector<std::uint32_t>{1}));
    const std::array star{Edge{0, 1}, Edge{0, 2}, Edge{0, 3}};
    assert((sirkit::tree_heap_orderings(4, star) == std::vector<std::uint32_t>{6, 2, 2, 2}));
    assert((sirkit::tree_heap_orderings(4, star, 5) == std::vector<std::uint32_t>{1, 2, 2, 2}));
    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::tree_heap_orderings(-1, {});
                                    });
    expect_error<std::invalid_argument>([]
                                        {
                                            (void)sirkit::tree_heap_orderings(0, {}, 1);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            (void)sirkit::tree_heap_orderings(3, {}, 3);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            (void)sirkit::tree_heap_orderings(2, {});
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array excess{Edge{0, 0}};
                                            (void)sirkit::tree_heap_orderings(0, excess);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array cycle{Edge{1, 2}, Edge{2, 3}, Edge{3, 1}};
                                            (void)sirkit::tree_heap_orderings(4, cycle);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array parallel{Edge{0, 1}, Edge{1, 0}};
                                            (void)sirkit::tree_heap_orderings(3, parallel);
                                        });
    for (const auto &[u, v] : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array invalid{Edge{u, v}};
                                            (void)sirkit::tree_heap_orderings(2, invalid);
                                        });
    }
    std::mt19937 random(160202611);
    for (int trial = 0; trial < 380; ++trial)
    {
        constexpr std::array<std::uint32_t, 3> kPrimes{13, 998244353, 4294967291U};
        const int n = static_cast<int>(random() % 11) + 1;
        std::vector<int> vertices(n);
        for (int v = 0; v < n; ++v)
        {
            vertices[v] = v;
        }
        std::shuffle(vertices.begin(), vertices.end(), random);
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            edges.emplace_back(vertices[v], vertices[random() % v]);
        }
        std::shuffle(edges.begin(), edges.end(), random);
        const auto prime = kPrimes[trial % kPrimes.size()];
        assert(sirkit::tree_heap_orderings(n, edges, prime) == brute(n, edges, prime));
    }

    constexpr int kSize = 200000;
    std::vector<Edge> chain;
    for (int v = 1; v < kSize; ++v)
    {
        chain.emplace_back(v - 1, v);
    }
    const auto counts = sirkit::tree_heap_orderings(kSize, chain);
    assert(counts.front() == 1 && counts.back() == 1);
    assert(counts[1] == kSize - 1 && counts[kSize - 2] == kSize - 1);
    const std::uint64_t select_two = static_cast<std::uint64_t>(kSize - 1) * (kSize - 2) / 2;
    assert(counts[2] == select_two % 1000000007);
    for (int v = 0; v < kSize; ++v)
    {
        assert(counts[v] == counts[kSize - 1 - v]);
    }
}
