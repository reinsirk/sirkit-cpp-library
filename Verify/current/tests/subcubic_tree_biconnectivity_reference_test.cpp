// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Subcubic_Tree_Biconnectivity.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <set>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

bool biconnected(int n, const std::vector<Edge> &tree, const std::vector<Edge> &extra)
{
    std::vector<std::vector<int>> adjacency(n);
    for (const auto *edges : {&tree, &extra})
    {
        for (const auto &[u, v] : *edges)
        {
            adjacency[u].push_back(v);
            adjacency[v].push_back(u);
        }
    }
    for (int cut = 0; cut < n; ++cut)
    {
        std::vector<bool> found(n);
        const int start = cut == 0 ? 1 : 0;
        std::vector<int> pending{start};
        found[start] = found[cut] = true;
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            for (const int w : adjacency[pending[i]])
            {
                if (!found[w])
                {
                    found[w] = true;
                    pending.push_back(w);
                }
            }
        }
        if (pending.size() != static_cast<std::size_t>(n - 1))
        {
            return false;
        }
    }
    return true;
}

std::uint64_t exhaustive_minimum(const std::vector<std::uint64_t> &weights, const std::vector<Edge> &tree,
                                 std::uint64_t known_cost)
{
    const int n = static_cast<int>(weights.size());
    std::set<Edge> original;
    for (const auto &[u, v] : tree)
    {
        original.emplace(std::min(u, v), std::max(u, v));
    }
    std::vector<Edge> missing;
    for (int u = 0; u < n; ++u)
    {
        for (int v = u + 1; v < n; ++v)
        {
            if (!original.contains({u, v}))
            {
                missing.emplace_back(u, v);
            }
        }
    }
    // The current optimum bound plus one still requires finding at least one witness independently.
    auto best = known_cost + 1;
    std::vector<Edge> chosen;
    auto search = [&](auto &&self, std::size_t next, std::uint64_t cost) -> void
    {
        if (cost >= best)
        {
            return;
        }
        if (biconnected(n, tree, chosen))
        {
            best = cost;
            return;
        }
        for (std::size_t i = next; i < missing.size(); ++i)
        {
            const auto [u, v] = missing[i];
            chosen.push_back(missing[i]);
            self(self, i + 1, cost + weights[u] + weights[v]);
            chosen.pop_back();
        }
    };
    search(search, 0, 0);
    return best;
}

auto verify(const std::vector<std::uint64_t> &weights, const std::vector<Edge> &edges, bool brute = false)
{
    const int n = static_cast<int>(weights.size());
    const auto answer = sirkit::augment_subcubic_tree_biconnectivity(weights, edges);
    assert(biconnected(n, edges, answer.edges));
    std::set<Edge> unique;
    for (const auto &[u, v] : edges)
    {
        unique.emplace(std::min(u, v), std::max(u, v));
    }
    unsigned __int128 total = 0;
    for (const auto &[u, v] : answer.edges)
    {
        assert(0 <= u && u < n && 0 <= v && v < n && u != v);
        assert(unique.emplace(std::min(u, v), std::max(u, v)).second);
        total += static_cast<unsigned __int128>(weights[u]) + weights[v];
    }
    if (total > std::numeric_limits<std::uint64_t>::max())
    {
        assert(!answer.cost);
    }
    else
    {
        assert(answer.cost == total);
        if (brute)
        {
            assert(exhaustive_minimum(weights, edges, answer.cost.value()) == answer.cost.value());
        }
    }
    return answer;
}

template <typename Error, typename Action>
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
    assert(verify({2, 3, 5}, {{0, 1}, {1, 2}}, true).cost == 7);
    assert(verify({1, 7, 7, 7}, {{0, 1}, {0, 2}, {0, 3}}, true).cost == 28);
    assert(verify({9, 1, 2, 8, 9}, {{0, 1}, {0, 2}, {0, 4}, {1, 3}}, true).cost == 20);
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    assert(verify({kMax - 1, 10, 1}, {{0, 1}, {1, 2}}).cost == kMax);
    assert(!verify({kMax - 1, 10, 3}, {{0, 1}, {1, 2}}).cost);
    assert(!verify({1, kMax, kMax, kMax}, {{0, 1}, {0, 2}, {0, 3}}).cost);

    // The antipode must not be the only leaf on its branch.
    const std::vector<Edge> branching{{0, 2}, {0, 1}, {0, 3}, {1, 4}, {1, 5}, {5, 6}, {5, 7}};
    assert(verify({0, 9, 2, 4, 8, 9, 16, 32}, branching).cost == 62);

    const std::vector<Edge> singleton_between{{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {3, 6}, {3, 7}};
    assert(verify({0, 9, 2, 4, 8, 9, 16, 32}, singleton_between).cost == 67);
    std::mt19937 random(160785);
    for (int trial = 0; trial < 1680; ++trial)
    {
        const int n = trial < 660 ? static_cast<int>(random() % 5) + 3 : static_cast<int>(random() % 120) + 3;
        std::vector<Edge> edges;
        std::vector<int> degree(n);
        std::vector<int> vertex(n);
        std::iota(vertex.begin(), vertex.end(), 0);
        std::shuffle(vertex.begin(), vertex.end(), random);
        for (int i = 1; i < n; ++i)
        {
            int j = 0;
            do
            {
                j = static_cast<int>(random() % i);
            } while (degree[j] == 3);
            edges.emplace_back(vertex[i], vertex[j]);
            ++degree[i];
            ++degree[j];
        }
        std::shuffle(edges.begin(), edges.end(), random);
        std::vector<std::uint64_t> weights(n);
        for (auto &weight : weights)
        {
            weight = random() % 20;
        }
        verify(weights, edges, trial < 660);
    }

    const auto limit = static_cast<std::size_t>(std::numeric_limits<int>::max());
    sirkit::detail::check_subcubic_tree_size(limit, limit - 1);
    expect_error<std::length_error>([&]
                                    {
                                        sirkit::detail::check_subcubic_tree_size(limit + 1, limit);
                                    });
    const std::vector<std::uint64_t> small(3), invalid(4), high_degree(5);
    const std::vector<Edge> path{{0, 1}, {1, 2}};
    expect_error<std::length_error>([&]
                                    {
                                        (void)sirkit::augment_subcubic_tree_biconnectivity({}, {});
                                    });
    expect_error<std::length_error>([&]
                                    {
                                        (void)sirkit::augment_subcubic_tree_biconnectivity(small, {});
                                    });
    const std::vector<Edge> star{{0, 1}, {0, 2}, {0, 3}, {0, 4}};
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)sirkit::augment_subcubic_tree_biconnectivity(high_degree, star);
                                        });
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &malformed : {Edge{-1, 0}, Edge{3, 0}, Edge{0, -1}, Edge{0, 3}})
    {
        auto wrong = path;
        wrong[0] = malformed;
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::augment_subcubic_tree_biconnectivity(small, wrong);
                                        });
    }
    const std::vector<Edge> cycle{{0, 1}, {1, 2}, {2, 0}}, disconnected{{1, 2}, {2, 3}, {3, 1}};
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)sirkit::augment_subcubic_tree_biconnectivity(invalid, cycle);
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)sirkit::augment_subcubic_tree_biconnectivity(invalid, disconnected);
                                        });
#endif
}
