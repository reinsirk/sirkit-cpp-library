// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Forest_Subset_Components.cpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;
using Weight = sirkit::ForestSubsetWeight<std::int64_t>;

std::vector<std::int64_t> oracle(
    int n, const std::vector<Edge> &edges, const std::vector<Weight> &weights)
{
    std::vector<std::int64_t> answer(n + 1);
    for (unsigned mask = 0; mask < (1U << n); ++mask)
    {
        int components = std::popcount(mask);
        for (const auto &[u, v] : edges)
        {
            components -= ((mask >> u) & 1U) && ((mask >> v) & 1U);
        }
        std::int64_t value = 1;
        if (!weights.empty())
        {
            for (int i = 0; i < n; ++i)
            {
                value *= ((mask >> i) & 1U) ? weights[i].take : weights[i].omit;
            }
        }
        answer[components] += value;
    }
    return answer;
}

void check(int n, const std::vector<Edge> &edges, const std::vector<Weight> &weights = {})
{
    assert(sirkit::forest_subset_components<std::int64_t>(n, edges, weights) == oracle(n, edges, weights));
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
    check(0, {});
    check(1, {});
    check(1, {}, {{0, 2}});
    check(5, {});
    check(6, {{0, 1}, {1, 2}, {3, 4}}, {{0, 1}, {2, 3}, {1, 3}, {2, -3}, {-1, 2}, {2, 0}});
    check(7, {{0, 1}, {0, 2}, {2, 3}, {2, 4}, {0, 5}, {5, 6}});
    check(7, {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}});

    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::forest_subset_components<int>(-1, {});
                                    });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array too_few{Weight{3, 4}};
                                            (void)sirkit::forest_subset_components<std::int64_t>(2, {}, too_few);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array self{Edge{0, 0}};
                                            (void)sirkit::forest_subset_components<int>(1, self);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array parallel{Edge{0, 1}, Edge{1, 0}};
                                            (void)sirkit::forest_subset_components<int>(2, parallel);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array cycle{Edge{0, 1}, Edge{1, 2}, Edge{2, 0}};
                                            (void)sirkit::forest_subset_components<int>(5, cycle);
                                        });
    for (const auto &[u, v] : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array bad{Edge{u, v}};
                                            (void)sirkit::forest_subset_components<int>(2, bad);
                                        });
    }

    std::mt19937 random(28753158);
    for (int trial = 0; trial < 420; ++trial)
    {
        const int n = static_cast<int>(random() % 12) + 1;
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            if (random() % 5 != 0)
            {
                edges.emplace_back(static_cast<int>(random() % v), v);
            }
        }
        std::shuffle(edges.begin(), edges.end(), random);
        std::vector<Weight> weights(n);
        for (auto &weight : weights)
        {
            weight = {static_cast<int>(random() % 9) - 4, static_cast<int>(random() % 9) - 4};
        }
        check(n, edges, weights);
        check(n, edges);
    }
}
