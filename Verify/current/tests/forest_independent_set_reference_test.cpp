// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Forest_Independent_Set.cpp"

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
using Weight = sirkit::ForestIndependentSetWeight<std::int64_t>;

struct Parity
{
    int value;
    explicit Parity(int count) : value(count & 1)
    {
    }
    friend Parity operator+(Parity a, Parity b)
    {
        return Parity(a.value ^ b.value);
    }
    friend Parity operator*(Parity a, Parity b)
    {
        return Parity(a.value & b.value);
    }
};

void compare(int n, const std::vector<Edge> &edges, const std::vector<Weight> &weights,
             const std::vector<std::int64_t> &reward)
{
    const sirkit::ForestIndependentSet forest(n, edges);
    std::int64_t total = 0, separate_count = 0, best = 0;
    std::vector<Weight> by_vertex(n);
    for (unsigned subset = 0; subset < (1U << n); ++subset)
    {
        bool independent = true;
        for (const auto &[u, v] : edges)
        {
            if (((subset >> u) & 1) && ((subset >> v) & 1))
            {
                independent = false;
                break;
            }
        }
        if (!independent)
        {
            continue;
        }

        ++separate_count;
        std::int64_t product = 1, score = 0;
        for (int v = 0; v < n; ++v)
        {
            if ((subset >> v) & 1)
            {
                product *= weights[v].included;
                score += reward[v];
            }
            else
            {
                product *= weights[v].excluded;
            }
        }
        total += product;
        best = std::max(best, score);
        for (int v = 0; v < n; ++v)
        {
            auto &value = (subset >> v) & 1 ? by_vertex[v].included : by_vertex[v].excluded;
            value += product;
        }
    }
    assert(forest.count<std::int64_t>() == separate_count);
    assert(forest.count<std::int64_t>(weights) == total);
    const auto seen = forest.count_by_vertex<std::int64_t>(weights);
    const auto unweighted = forest.count_by_vertex<std::int64_t>();
    assert(seen.size() == weights.size() && unweighted.size() == weights.size());
    for (int v = 0; v < n; ++v)
    {
        assert(seen[v].excluded == by_vertex[v].excluded);
        assert(seen[v].included == by_vertex[v].included);
        assert(unweighted[v].excluded + unweighted[v].included == separate_count);
    }

    const auto optimum = forest.maximum_weight<std::int64_t>(reward);
    assert(optimum.weight == best);
    std::vector<bool> selected(n);
    std::int64_t score = 0;
    for (const int v : optimum.vertices)
    {
        assert(0 <= v && v < n && !selected[v]);
        selected[v] = true;
        score += reward[v];
    }
    for (const auto &[u, v] : edges)
    {
        assert(!selected[u] || !selected[v]);
    }
    assert(score == best);
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
    compare(0, {}, {}, {});
    compare(1, {}, {{0, 0}}, {-5});
    compare(4, {{0, 1}, {0, 2}, {0, 3}}, {{0, -2}, {1, 2}, {3, 3}, {-1, 2}}, {7, 2, 3, 4});
    compare(5, {{0, 1}, {2, 4}}, {{2, 1}, {1, 2}, {0, 4}, {-1, 1}, {2, 5}}, {0, 1, -2, 0, 4});

    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::ForestIndependentSet(-1, {});
                                    });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array excess{Edge{0, 0}};
                                            (void)sirkit::ForestIndependentSet(0, excess);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array self{Edge{0, 0}};
                                            (void)sirkit::ForestIndependentSet(1, self);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array parallel{Edge{0, 1}, Edge{1, 0}};
                                            (void)sirkit::ForestIndependentSet(3, parallel);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array cycle{Edge{1, 2}, Edge{2, 3}, Edge{3, 1}};
                                            (void)sirkit::ForestIndependentSet(4, cycle);
                                        });
    for (const auto &[u, v] : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array invalid{Edge{u, v}};
                                            (void)sirkit::ForestIndependentSet(2, invalid);
                                        });
    }
    const sirkit::ForestIndependentSet isolated(2, {});
    expect_error<std::invalid_argument>([&]
                                        {
                                            const std::array invalid{Weight{1, 2}};
                                            (void)isolated.count<std::int64_t>(invalid);
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)isolated.maximum_weight<int>({});
                                        });

    std::mt19937 random(11052026);
    for (int trial = 0; trial < 400; ++trial)
    {
        const int n = static_cast<int>(random() % 9) + 1;
        std::vector<int> vertices(n);
        for (int v = 0; v < n; ++v)
        {
            vertices[v] = v;
        }
        std::shuffle(vertices.begin(), vertices.end(), random);
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            if (random() % 5 != 0)
            {
                edges.emplace_back(vertices[v], vertices[random() % v]);
            }
        }
        std::shuffle(edges.begin(), edges.end(), random);
        std::vector<Weight> weights(n);
        std::vector<std::int64_t> reward(n);
        for (int v = 0; v < n; ++v)
        {
            weights[v] = {static_cast<int>(random() % 6) - 2, static_cast<int>(random() % 6) - 2};
            reward[v] = static_cast<int>(random() % 13) - 6;
        }
        compare(n, edges, weights, reward);
    }

    constexpr int kSize = 100000;
    std::vector<Edge> edges;
    for (int v = 1; v < kSize; ++v)
    {
        edges.emplace_back(v - 1, v);
    }
    const sirkit::ForestIndependentSet chain(kSize, edges);
    const std::vector<int> weights(kSize, 1);
    assert(chain.maximum_weight<int>(weights).weight == kSize / 2);
    assert(chain.count<Parity>().value == 0);
    for (auto &edge : edges)
    {
        edge.first = 0;
    }
    const sirkit::ForestIndependentSet star(kSize, edges);
    assert(star.count<Parity>().value == 1);
    const auto conditional = star.count_by_vertex<Parity>();
    assert(conditional[0].excluded.value == 0 && conditional[0].included.value == 1);
    for (int v = 1; v < kSize; ++v)
    {
        assert(conditional[v].excluded.value == 1 && conditional[v].included.value == 0);
    }
}
