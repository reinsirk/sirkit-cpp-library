// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Core_Decomposition.cpp"

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
using Edge = std::pair<int, int>;

template <class T>
std::vector<T> brute(int n, const std::vector<Edge> &edges, const std::vector<T> &weights)
{
    std::vector<T> expected(n);
    for (unsigned subset = 1; subset < (1U << n); ++subset)
    {
        std::vector<T> degree(n);
        for (const auto &[u, v] : edges)
        {
            if ((subset >> u & 1U) && (subset >> v & 1U))
            {
                degree[u] += weights[v];
                degree[v] += weights[u];
            }
        }
        T threshold = std::numeric_limits<T>::max();
        for (int v = 0; v < n; ++v)
        {
            if (subset >> v & 1U)
            {
                threshold = std::min(threshold, degree[v]);
            }
        }
        for (int v = 0; v < n; ++v)
        {
            if (subset >> v & 1U)
            {
                expected[v] = std::max(expected[v], threshold);
            }
        }
    }
    return expected;
}

template <class T>
void check(
    int n, const std::vector<Edge> &edges, const std::vector<T> &weights,
    const sirkit::CoreDecompositionResult<T> &result)
{
    const auto expected = brute(n, edges, weights);
    assert(result.number == expected);
    assert(result.order.size() == expected.size());
    const T greatest = n == 0 ? T{0} : *std::max_element(expected.begin(), expected.end());
    assert(result.degeneracy == greatest);
    T previous{0};
    std::vector<bool> removed(n);
    for (const int v : result.order)
    {
        assert(0 <= v && v < n && !removed[v]);
        assert(previous <= result.number[v]);
        previous = result.number[v];
        T actual{0};
        for (const auto &[u, w] : edges)
        {
            if (u == v && !removed[w])
            {
                actual += weights[w];
            }
            if (w == v && !removed[u])
            {
                actual += weights[u];
            }
        }
        assert(actual <= result.number[v]);
        removed[v] = true;
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
    check<std::size_t>(0, {}, {}, sirkit::core_decomposition(0, {}));
    check<int>(0, {}, {}, sirkit::weighted_core_decomposition<int>(0, {}, {}));
    const std::vector<Edge> parallel{{0, 1}, {1, 0}, {1, 2}, {2, 1}, {1, 2}};
    const std::vector<std::uint64_t> unsigned_weights{0, 9, 4};
    check<std::size_t>(3, parallel, {1, 1, 1}, sirkit::core_decomposition(3, parallel));
    check(3, parallel, unsigned_weights, sirkit::weighted_core_decomposition<std::uint64_t>(3, parallel, unsigned_weights));
    const std::array sole{Edge{0, 1}};
    constexpr auto largest = std::numeric_limits<std::int64_t>::max();
    constexpr std::array maximal{largest, largest};
    assert(sirkit::weighted_core_decomposition<std::int64_t>(2, sole, maximal).degeneracy == largest);

    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::core_decomposition(-1, {});
                                    });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array self{Edge{0, 0}};
                                            (void)sirkit::core_decomposition(1, self);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            (void)sirkit::weighted_core_decomposition<int>(1, {}, {});
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            constexpr std::array bad{-1};
                                            (void)sirkit::weighted_core_decomposition<int>(1, {}, bad);
                                        });
    expect_error<std::overflow_error>([]
                                      {
                                          const std::array path{Edge{0, 1}, Edge{1, 2}};
                                          constexpr std::array weights{largest, std::int64_t{2}, std::int64_t{1}};
                                          (void)sirkit::weighted_core_decomposition<std::int64_t>(3, path, weights);
                                      });
    for (const auto &[u, v] : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array invalid{Edge{u, v}};
                                            (void)sirkit::core_decomposition(2, invalid);
                                        });
    }

    std::mt19937 random(267402003);
    for (int trial = 0; trial < 500; ++trial)
    {
        const int n = static_cast<int>(random() % 9) + 1;
        std::vector<Edge> edges;
        std::vector<int> weights(n);
        for (int v = 0; v < n; ++v)
        {
            weights[v] = static_cast<int>(random() % 12);
            for (int u = 0; u < v; ++u)
            {
                const int copies = static_cast<int>(random() % 4);
                for (int copy = 0; copy < copies; ++copy)
                {
                    edges.emplace_back(u, v);
                }
            }
        }
        std::shuffle(edges.begin(), edges.end(), random);
        check(n, edges, weights, sirkit::weighted_core_decomposition<int>(n, edges, weights));
        check(n, edges, std::vector<std::size_t>(n, 1), sirkit::core_decomposition(n, edges));
    }

    constexpr int kSize = 100000;
    std::vector<Edge> edges;
    for (int v = 0; v < kSize; ++v)
    {
        edges.emplace_back(v, (v + 1) % kSize);
        edges.emplace_back(v, (v + 2) % kSize);
    }
    const auto unweighted = sirkit::core_decomposition(kSize, edges);
    assert(unweighted.degeneracy == 4);
    assert(std::ranges::all_of(unweighted.number, [](std::size_t k)
                               {
                                   return k == 4;
                               }));
    const std::vector<std::uint64_t> weights(kSize, 1000000000);
    const auto weighted = sirkit::weighted_core_decomposition<std::uint64_t>(kSize, edges, weights);
    assert(weighted.degeneracy == 4000000000ULL);
    assert(std::ranges::all_of(weighted.number, [](std::uint64_t k)
                               {
                                   return k == 4000000000ULL;
                               }));
}
