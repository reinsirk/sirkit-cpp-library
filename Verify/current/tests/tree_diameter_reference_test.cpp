// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TREE_DIAMETER_TEST_SOURCE
#define SIRKIT_TREE_DIAMETER_TEST_SOURCE "../../../Graph/Tree_Diameter.cpp"
#endif
#include SIRKIT_TREE_DIAMETER_TEST_SOURCE
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::TreeDiameterEdge;

void CompareWithAllPairs(int n, const std::vector<Edge> &edges)
{
    constexpr std::int64_t kUnreachable = 1'000'000;
    std::vector distances(n, std::vector<std::int64_t>(n, kUnreachable));
    for (int i = 0; i < n; ++i)
    {
        distances[i][i] = 0;
    }
    for (const auto &[from, to, weight] : edges)
    {
        distances[from][to] = weight;
        distances[to][from] = weight;
    }
    auto weights = distances;
    for (int via = 0; via < n; ++via)
    {
        for (int from = 0; from < n; ++from)
        {
            for (int to = 0; to < n; ++to)
            {
                distances[from][to] = std::min(distances[from][to], distances[from][via] + distances[via][to]);
            }
        }
    }

    std::int64_t expected = 0;
    for (const auto &row : distances)
    {
        expected = std::max(expected, *std::max_element(row.begin(), row.end()));
    }

    const auto result = sirkit::tree_diameter(n, edges);
    assert(result.length == expected);
    assert(!result.path.empty());
    std::vector<bool> visited(n);
    std::int64_t cost = 0;
    for (std::size_t i = 0; i < result.path.size(); ++i)
    {
        const int vertex = result.path[i];
        assert(0 <= vertex && vertex < n);
        assert(!visited[vertex]);
        visited[vertex] = true;
        if (i != 0)
        {
            const auto weight = weights[result.path[i - 1]][vertex];
            assert(weight != kUnreachable);
            cost += weight;
        }
    }
    assert(cost == expected);
}

template <class Exception, class Function>
void ExpectException(Function operation)
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
    const auto empty = sirkit::tree_diameter(0, {});
    assert(empty.length == 0 && empty.path.empty());

    const auto singleton = sirkit::tree_diameter(1, {});
    assert(singleton.length == 0 && singleton.path == std::vector{0});

    const auto zero = sirkit::tree_diameter(4, std::array{Edge{2, 1, 0}, Edge{2, 0, 0}, Edge{1, 3, 0}});
    assert(zero.length == 0 && zero.path == std::vector{0});

    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    const auto limit = sirkit::tree_diameter(3, std::array{Edge{1, 0, kMax - 1}, Edge{1, 2, 1}});
    assert(limit.length == kMax && limit.path.size() == 3);

    // 頂点0からの距離は収まるが、2つの葉を結ぶ直径が64ビットの上限を超える。
    constexpr auto kHighWeight = kMax / 2 + 1;
    ExpectException<std::overflow_error>([]
                                         {
                                             sirkit::tree_diameter(3, std::array{Edge{0, 1, kHighWeight}, Edge{0, 2, kHighWeight}});
                                         });

#ifndef SKIP_RUNTIME_CHECKS
    ExpectException<std::invalid_argument>([]
                                           {
                                               sirkit::tree_diameter(-1, {});
                                           });
    ExpectException<std::invalid_argument>([]
                                           {
                                               sirkit::tree_diameter(3, std::array{Edge{0, 1, 0}});
                                           });
    ExpectException<std::invalid_argument>([]
                                           {
                                               sirkit::tree_diameter(0, std::array{Edge{0, 1, 0}});
                                           });
    ExpectException<std::out_of_range>([]
                                       {
                                           sirkit::tree_diameter(2, std::array{Edge{-1, 0, 0}});
                                       });
    ExpectException<std::out_of_range>([]
                                       {
                                           sirkit::tree_diameter(2, std::array{Edge{0, 2, 0}});
                                       });
    ExpectException<std::invalid_argument>([]
                                           {
                                               sirkit::tree_diameter(2, std::array{Edge{0, 1, -1}});
                                           });
    ExpectException<std::invalid_argument>([]
                                           {
                                               sirkit::tree_diameter(2, std::array{Edge{0, 0, 0}});
                                           });
    ExpectException<std::invalid_argument>([]
                                           {
                                               sirkit::tree_diameter(3, std::array{Edge{0, 1, 1}, Edge{1, 0, 2}});
                                           });
    ExpectException<std::invalid_argument>([]
                                           {
                                               sirkit::tree_diameter(3, std::array{Edge{2, 1, 1}, Edge{1, 2, 2}});
                                           });
#endif

    std::mt19937 rng(0x31d1a13U);
    for (int n = 1; n <= 24; ++n)
    {
        for (int run = 0; run < 30; ++run)
        {
            std::vector<Edge> edges;
            for (int v = 1; v < n; ++v)
            {
                const auto p = static_cast<int>(rng() % v);
                const auto weight = static_cast<std::int64_t>(rng() % (run % 8 + 1));
                edges.push_back({p, v, weight});
                if (rng() % 2 != 0)
                {
                    std::swap(edges.back().from, edges.back().to);
                }
            }
            std::shuffle(edges.begin(), edges.end(), rng);
            CompareWithAllPairs(n, edges);
        }
    }

    constexpr int kChainSize = 500'000;
    constexpr std::int64_t kChainWeight = 1'000'000'000;
    std::vector<Edge> chain;
    chain.reserve(kChainSize - 1);
    for (int v = 1; v < kChainSize; ++v)
    {
        chain.push_back({v, v - 1, kChainWeight});
    }
    const auto result = sirkit::tree_diameter(kChainSize, chain);
    assert(result.length == (kChainSize - 1) * kChainWeight);
    assert(result.path.size() == kChainSize);
    const bool forward = result.path.front() == 0;
    for (int v = 0; v < kChainSize; ++v)
    {
        assert(result.path[v] == (forward ? v : kChainSize - v - 1));
    }
}
