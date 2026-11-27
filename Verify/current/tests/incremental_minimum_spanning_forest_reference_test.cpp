// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_INCREMENTAL_MINIMUM_SPANNING_FOREST_DEFINED
#include "../../../DataStructure/Link_Cut_Tree.cpp"
#include "../../../Graph/Incremental_Minimum_Spanning_Forest.cpp"
#endif

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Forest = sirkit::IncrementalMinimumSpanningForest;

struct Edge
{
    int from;
    int to;
    std::int64_t weight;
};

struct CheckForest
{
    std::vector<int> leader;
    int components;
    std::int64_t weight = 0;
    explicit CheckForest(int n) : leader(n), components(n)
    {
        std::iota(leader.begin(), leader.end(), 0);
    }

    bool add(const Edge &edge)
    {
        const int a = leader[edge.from], b = leader[edge.to];
        if (a == b)
        {
            return false;
        }
        for (auto &v : leader)
        {
            if (v == b)
            {
                v = a;
            }
        }
        --components;
        weight += edge.weight;
        return true;
    }
};

void check(int n, const std::vector<Edge> &edges, bool unique)
{
    Forest forest(n);
    std::vector<bool> retained;
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &edge = edges[i];
        retained.push_back(false);
        const auto update = forest.add_edge(edge.from, edge.to, edge.weight);
        assert(update.edge_id == i && forest.edge_count() == i + 1);
        if (update.selected)
        {
            retained[i] = true;
            if (update.removed_edge)
            {
                const auto removed = *update.removed_edge;
                assert(removed < i && retained[removed] && edges[removed].weight > edge.weight);
                retained[removed] = false;
            }
        }
        else
        {
            assert(!update.removed_edge);
        }

        std::vector<std::size_t> order(i + 1);
        std::iota(order.begin(), order.end(), std::size_t{0});
        std::sort(order.begin(), order.end(), [&](auto a, auto b)
                  {
                      return std::pair{edges[a].weight, a} < std::pair{edges[b].weight, b};
                  });
        CheckForest kruskal(n), recovered(n);
        for (auto id : order)
        {
            const bool reference = kruskal.add(edges[id]);
            if (retained[id])
            {
                assert(recovered.add(edges[id]));
            }
            if (unique)
            {
                assert(reference == retained[id]);
            }
        }
        assert(forest.size() == n && forest.component_count() == kruskal.components);
        assert(recovered.components == kruskal.components);
        assert(forest.total_weight() == kruskal.weight && recovered.weight == kruskal.weight);
    }
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
    const Forest empty(0);
    assert(empty.total_weight() == 0 && empty.size() == 0 && empty.component_count() == 0);
    check(4, {{0, 1, 7}, {1, 2, 7}, {2, 3, 7}, {0, 3, 7}, {0, 3, -1}, {1, 2, -9}, {0, 0, -15}}, false);
    check(2, {{0, 0, -9}, {0, 1, 4}, {1, 0, -3}, {0, 1, 1}}, true);

    std::mt19937 random(51445);
    for (int trial = 0; trial < 1400; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 13);
        const auto m = random() % 65;
        const bool unique = (trial % 2) == 0;
        std::vector<Edge> edges;
        std::vector<std::int64_t> weights(m);
        std::iota(weights.begin(), weights.end(), std::int64_t{-32});
        std::shuffle(weights.begin(), weights.end(), random);
        for (std::size_t i = 0; i < m; ++i)
        {
            const int from = static_cast<int>(random() % n), to = static_cast<int>(random() % n);
            const auto weight = unique ? weights[i] : static_cast<std::int64_t>(random() % 31) - 15;
            edges.push_back({from, to, weight});
        }
        check(n, edges, unique);
    }

    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    Forest limits(3);
    assert(limits.add_edge(0, 1, kMax).selected);
    assert(limits.total_weight() == kMax);
    assert(limits.add_edge(1, 2, kMax).selected);
    expect_error<std::overflow_error>([&]
                                      {
                                          static_cast<void>(limits.total_weight());
                                      });
    const auto replace = limits.add_edge(0, 2, kMin);
    assert(replace.selected && replace.removed_edge && limits.total_weight() == -1);
    const auto parallel = limits.add_edge(2, 0, kMin);
    assert(!parallel.selected && !parallel.removed_edge);

    Forest below(5);
    assert(below.add_edge(0, 1, kMin).selected);
    assert(below.total_weight() == kMin);
    assert(below.add_edge(1, 2, kMin).selected);
    expect_error<std::overflow_error>([&]
                                      {
                                          static_cast<void>(below.total_weight());
                                      });
    assert(below.add_edge(2, 3, kMax).selected);
    expect_error<std::overflow_error>([&]
                                      {
                                          static_cast<void>(below.total_weight());
                                      });
    assert(below.add_edge(3, 4, kMax).selected && below.total_weight() == -2);

    Forest copied = limits;
    const int kept = replace.removed_edge == 0 ? 1 : 0;
    assert(copied.add_edge(kept, kept + 1, 0).edge_id == 4);
    assert(limits.edge_count() == 4 && limits.total_weight() == -1);
    Forest moved = std::move(copied);
    assert(moved.edge_count() == 5 && moved.total_weight() == kMin);
    assert(copied.edge_count() == 0 && copied.component_count() == 0 && copied.size() == 0 &&
           copied.total_weight() == 0);
    copied = below;
    assert(copied.edge_count() == 4 && copied.total_weight() == -2);
    copied = std::move(moved);
    assert(copied.edge_count() == 5 && copied.total_weight() == kMin);
    assert(moved.size() == 0 && moved.edge_count() == 0);

    expect_error<std::length_error>([]
                                    {
                                        static_cast<void>(Forest(-1));
                                    });
    expect_error<std::length_error>([]
                                    {
                                        static_cast<void>(Forest(std::numeric_limits<int>::max()));
                                    });
    auto next = std::numeric_limits<std::size_t>::max() - 1;
    assert(sirkit::detail::take_incremental_msf_edge_id(next) == std::numeric_limits<std::size_t>::max() - 1);
    expect_error<std::overflow_error>([&]
                                      {
                                          static_cast<void>(sirkit::detail::take_incremental_msf_edge_id(next));
                                      });
#ifndef SKIP_RUNTIME_CHECKS
    Forest checked(2);
    for (const auto &[from, to] : {std::pair{-1, 0}, std::pair{2, 0}, std::pair{0, -1}, std::pair{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            static_cast<void>(checked.add_edge(from, to, 2));
                                        });
    }
    assert(checked.edge_count() == 0 && checked.total_weight() == 0 && checked.component_count() == 2);
#endif

    constexpr int kSize = 25000;
    constexpr int kQueries = 60000;
    Forest cycle(kSize);
    for (int i = 0; i < kQueries; ++i)
    {
        const auto result = cycle.add_edge(i % kSize, (i + 1) % kSize, kQueries - i);
        assert(result.selected && result.edge_id == static_cast<std::size_t>(i));
        if (i < kSize - 1)
        {
            assert(!result.removed_edge);
        }
        else
        {
            assert(result.removed_edge == static_cast<std::size_t>(i - kSize + 1));
        }
    }
}
