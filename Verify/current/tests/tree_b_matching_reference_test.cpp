// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_B_Matching.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Edge = sirkit::TreeBMatchingEdge;
using sirkit::maximum_weight_tree_b_matching;

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

std::int64_t brute(const std::vector<int> &cap, const std::vector<Edge> &edges)
{
    std::int64_t best = 0;
    for (unsigned mask = 0; mask < (1U << edges.size()); ++mask)
    {
        std::vector<int> degree(cap.size());
        std::int64_t weight = 0;
        bool valid = true;
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            if (((mask >> i) & 1U) != 0)
            {
                const auto &[from, to, value] = edges[i];
                ++degree[from];
                ++degree[to];
                valid &= degree[from] <= cap[from] && degree[to] <= cap[to];
                weight += value;
            }
        }
        if (valid)
        {
            best = std::max(best, weight);
        }
    }
    return best;
}

void check(const std::vector<int> &cap, const std::vector<Edge> &edges, std::int64_t expected)
{
    const auto result = maximum_weight_tree_b_matching(cap, edges);
    assert(result.weight == expected);
    std::vector<bool> seen(edges.size());
    std::vector<int> degree(cap.size());
    std::int64_t sum = 0;
    for (const auto id : result.edge_ids)
    {
        assert(id < edges.size() && !seen[id]);
        seen[id] = true;
        const auto &[from, to, weight] = edges[id];
        assert(weight > 0);
        ++degree[from];
        ++degree[to];
        assert(degree[from] <= cap[from] && degree[to] <= cap[to]);
        sum += weight;
    }
    assert(sum == expected);
}
} // namespace

int main()
{
    check({}, {}, 0);
    check({0}, {}, 0);
    check({999}, {}, 0);
    check({1, 1, 1}, {{0, 1, 4}, {1, 2, 7}}, 7);
    check({1, 1, 1}, {{0, 1, 9}, {1, 2, 7}}, 9);
    check({0, 1, 1}, {{0, 1, 9}, {1, 2, 7}}, 7);
    check({1, 0, 1}, {{0, 1, 9}, {1, 2, 7}}, 0);
    check({2, 1, 1, 1}, {{0, 1, 4}, {0, 2, 4}, {0, 3, 4}}, 8);
    check({9, 9, 9}, {{0, 1, 4}, {1, 2, 7}}, 11);

    std::mt19937 random(148023);
    for (int trial = 0; trial < 780; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 11);
        std::vector<int> cap(n);
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            edges.push_back({v, static_cast<int>(random() % v), static_cast<std::int64_t>(random() % 41) - 20});
        }
        for (int &value : cap)
        {
            value = static_cast<int>(random() % 5);
        }
        std::shuffle(edges.begin(), edges.end(), random);
        check(cap, edges, brute(cap, edges));
    }

    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    check({1, 1, 1}, {{0, 1, kMax}, {1, 2, kMax}}, kMax);
    check({2, 2, 2}, {{0, 1, kMin}, {1, 2, kMax}}, kMax);
    check({2, 2, 2}, {{0, 1, 1}, {1, 2, kMax - 1}}, kMax);
    check({2, 1, 1, 1}, {{0, 1, 5}, {0, 2, kMin}, {0, 3, 7}}, 12);
    const std::array high{Edge{0, 1, kMax}, Edge{1, 2, 1}};
    const std::array sufficient{2, 2, 2};
    expect_error<std::overflow_error>([&]
                                      {
                                          static_cast<void>(maximum_weight_tree_b_matching(sufficient, high));
                                      });
    const std::array bad_count{1, 1};
    expect_error<std::invalid_argument>([&]
                                        {
                                            static_cast<void>(maximum_weight_tree_b_matching(bad_count, {}));
                                        });
#ifndef SKIP_RUNTIME_CHECKS
    const std::array negative{-1};
    expect_error<std::invalid_argument>([&]
                                        {
                                            static_cast<void>(maximum_weight_tree_b_matching(negative, {}));
                                        });
    for (Edge invalid : {Edge{-1, 0, 0}, Edge{2, 0, 0}, Edge{0, -1, 0}, Edge{0, 2, 0}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            static_cast<void>(maximum_weight_tree_b_matching(bad_count, {&invalid, 1}));
                                        });
    }
    const std::array disconnected{Edge{0, 1, 4}, Edge{1, 0, 5}};
    expect_error<std::invalid_argument>([&]
                                        {
                                            static_cast<void>(maximum_weight_tree_b_matching(sufficient, disconnected));
                                        });
#endif
}
