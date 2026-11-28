// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Subsequence_Shortest_Paths.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Cost = std::int64_t;
using Edge = sirkit::SubsequencePathEdge<Cost>;
using Paths = sirkit::SubsequenceShortestPaths<Cost>;

void compare(int n, const std::vector<Edge> &edges, const std::vector<int> &schedule, int source)
{
    const Paths paths(n, edges, schedule, source);
    std::vector<std::optional<Cost>> brute(n);
    for (unsigned state = 0; state < (1U << schedule.size()); ++state)
    {
        int v = source;
        Cost cost = 0;
        bool valid = true;
        for (std::size_t i = 0; i < schedule.size(); ++i)
        {
            if (state >> i & 1U)
            {
                const auto &edge = edges[schedule[i]];
                if (edge.from != v)
                {
                    valid = false;
                    break;
                }
                v = edge.to;
                cost += edge.weight;
            }
        }
        if (valid && (!brute[v] || cost < *brute[v]))
        {
            brute[v] = cost;
        }
    }
    const auto distances = paths.distances();
    assert(distances.size() == brute.size());
    for (int target = 0; target < n; ++target)
    {
        assert(distances[target] == brute[target]);
        assert(paths.distance_to(target) == brute[target]);
        const auto witness = paths.path_to(target);
        assert(witness.has_value() == brute[target].has_value());
        if (!witness)
        {
            continue;
        }
        Cost sum = 0;
        int v = source;
        std::size_t next_valid = 0;
        for (const auto &[at, id] : *witness)
        {
            assert(next_valid <= at && at < schedule.size() && schedule[at] == id);
            assert(edges[id].from == v);
            v = edges[id].to;
            sum += edges[id].weight;
            next_valid = at + 1;
        }
        assert(v == target && sum == *brute[target]);
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
    compare(1, {}, {}, 0);
    compare(4, {}, {}, 2);
    compare(3, {{0, 1, 10}, {1, 2, 0}, {0, 1, 1}}, {0, 1, 2}, 0);
    compare(3, {{0, 0, -5}, {0, 1, -2}, {1, 0, -3}, {1, 2, 0}}, {0, 0, 1, 2, 0, 1, 3, 2}, 0);
    compare(2, {{0, 1, 4}, {0, 1, 4}, {0, 1, 6}}, {0, 1, 2}, 0);
    compare(4, {{0, 1, 4}, {1, 2, -3}, {2, 3, 0}}, {2, 1, 0}, 0);

    expect_error<std::length_error>([]
                                    {
                                        (void)Paths(0, {}, {}, 0);
                                    });
    for (const int v : {-1, 2})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)Paths(2, {}, {}, v);
                                        });
        const Paths paths(2, {}, {}, 0);
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)paths.path_to(v);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array input{v == -1 ? -1 : 1};
                                            const std::array edges{Edge{0, 1, 0}};
                                            (void)Paths(2, edges, input, 0);
                                        });
    }
    const std::array invalid_edges{Edge{-1, 0, 0}, Edge{2, 0, 0}, Edge{0, -1, 0}, Edge{0, 2, 0}};
    for (const auto &invalid : invalid_edges)
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array edges{invalid};
                                            (void)Paths(2, edges, {}, 0);
                                        });
    }
    constexpr Cost min_cost = std::numeric_limits<Cost>::lowest();
    constexpr Cost max_cost = std::numeric_limits<Cost>::max();
    const std::array first{0};
    const std::array two{0, 1};
    for (const auto extreme : {min_cost, max_cost})
    {
        const std::array edges{Edge{0, 1, extreme}, Edge{1, 2, extreme < 0 ? -1 : 1}};
        assert(Paths(3, edges, first, 0).distance_to(1).value() == extreme);
        expect_error<std::overflow_error>([&]
                                          {
                                              (void)Paths(3, edges, two, 0);
                                          });
    }

    using UnsignedEdge = sirkit::SubsequencePathEdge<std::uint64_t>;
    const std::array max_unsigned{UnsignedEdge{0, 1, std::numeric_limits<std::uint64_t>::max()},
                                  UnsignedEdge{1, 2, 1}};
    assert(sirkit::SubsequenceShortestPaths<std::uint64_t>(3, max_unsigned, first, 0).distance_to(1) == std::numeric_limits<std::uint64_t>::max());
    expect_error<std::overflow_error>([&]
                                      {
                                          (void)sirkit::SubsequenceShortestPaths<std::uint64_t>(3, max_unsigned, two, 0);
                                      });

    std::mt19937 random(2718844);
    for (int trial = 0; trial < 360; ++trial)
    {
        const int n = static_cast<int>(random() % 7) + 1;
        std::vector<Edge> edges(random() % 10 + 1);
        std::vector<int> schedule(random() % 12);
        for (auto &edge : edges)
        {
            edge = {static_cast<int>(random() % n), static_cast<int>(random() % n),
                    static_cast<int>(random() % 27) - 13};
        }
        for (auto &id : schedule)
        {
            id = static_cast<int>(random() % edges.size());
        }
        compare(n, edges, schedule, static_cast<int>(random() % n));
    }

    constexpr int kSize = 200000;
    std::vector<Edge> chain;
    std::vector<int> schedule;
    for (int i = 0; i < kSize - 1; ++i)
    {
        chain.push_back({i, i + 1, 1000000000});
        schedule.push_back(i);
    }
    const Paths deep(kSize, chain, schedule, 0);
    assert(deep.distance_to(kSize - 1) == (kSize - 1) * Cost{1000000000});
    const auto witness = deep.path_to(kSize - 1).value();
    assert(witness.size() == kSize - 1);
    for (std::size_t i = 0; i < witness.size(); ++i)
    {
        assert(witness[i].occurrence == i && witness[i].edge_id == static_cast<int>(i));
    }

    const std::array loop{Edge{0, 0, -1}};
    const std::vector<int> repeated(kSize);
    const Paths looping(1, loop, repeated, 0);
    assert(looping.distance_to(0) == -kSize);
    assert(looping.path_to(0)->size() == kSize);
}
