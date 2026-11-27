// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Steiner_Tree.cpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
constexpr auto kMax = std::numeric_limits<std::int64_t>::max();

struct BruteAnswer
{
    bool connected = false;
    std::optional<std::int64_t> cost;
};

BruteAnswer brute_force(int n, const std::vector<sirkit::SteinerTreeEdge> &edges, const std::vector<int> &terminals)
{
    if (terminals.empty())
    {
        return {true, 0};
    }
    BruteAnswer answer;
    for (unsigned subset = 0; subset < (1U << edges.size()); ++subset)
    {
        std::vector<bool> reached(n);
        reached[terminals.front()] = true;
        bool changed = true;
        while (changed)
        {
            changed = false;
            for (std::size_t id = 0; id < edges.size(); ++id)
            {
                if ((subset >> id & 1U) == 0)
                {
                    continue;
                }
                const auto &edge = edges[id];
                if (reached[edge.from] != reached[edge.to])
                {
                    reached[edge.from] = reached[edge.to] = true;
                    changed = true;
                }
            }
        }
        if (!std::ranges::all_of(terminals, [&](int v)
                                 {
                                     return static_cast<bool>(reached[v]);
                                 }))
        {
            continue;
        }

        answer.connected = true;
        std::int64_t sum = 0;
        bool fits = true;
        for (std::size_t id = 0; id < edges.size(); ++id)
        {
            if ((subset >> id & 1U) == 0)
            {
                continue;
            }
            if (edges[id].weight > kMax - sum)
            {
                fits = false;
                break;
            }
            sum += edges[id].weight;
        }
        if (fits && (!answer.cost || sum < *answer.cost))
        {
            answer.cost = sum;
        }
    }
    return answer;
}

void check_tree(int n, std::span<const sirkit::SteinerTreeEdge> edges, std::span<const int> terminals,
                const sirkit::SteinerTree &answer)
{
    std::vector<int> label(n);
    std::iota(label.begin(), label.end(), 0);
    std::int64_t sum = 0;
    for (auto id : answer.edge_ids)
    {
        assert(id < edges.size());
        const auto &[from, to, weight] = edges[id];
        assert(label[from] != label[to]);
        const int previous = label[to], replacement = label[from];
        for (int &current : label)
        {
            if (current == previous)
            {
                current = replacement;
            }
        }
        assert(weight <= kMax - sum);
        sum += weight;
    }
    assert(sum == answer.cost);
    if (!terminals.empty())
    {
        const auto component = label[terminals.front()];
        for (auto terminal : terminals)
        {
            assert(label[terminal] == component);
        }
        for (auto id : answer.edge_ids)
        {
            assert(label[edges[id].from] == component);
        }
    }
    else
    {
        assert(answer.edge_ids.empty());
    }
}

template <class Exception, class Run>
void expect_error(Run run)
{
    try
    {
        run();
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
} // namespace

int main()
{
    const auto empty = sirkit::minimum_steiner_tree(0, {}, {});
    assert(empty && empty->cost == 0 && empty->edge_ids.empty());
    const std::array duplicate{1, 1, 1};
    const auto single = sirkit::minimum_steiner_tree(2, {}, duplicate);
    assert(single && single->cost == 0 && single->edge_ids.empty());

    const std::array<sirkit::SteinerTreeEdge, 2> wide{{{0, 1, kMax - 41}, {1, 2, 41}}};
    const std::array endpoints{0, 2};
    const auto boundary = sirkit::minimum_steiner_tree(3, wide, endpoints);
    assert(boundary && boundary->cost == kMax);
    check_tree(3, wide, endpoints, *boundary);

    std::mt19937 random(591347);
    for (int trial = 0; trial < 1400; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 7);
        std::vector<sirkit::SteinerTreeEdge> edges(random() % 11);
        for (auto &edge : edges)
        {
            edge.from = static_cast<int>(random() % n);
            edge.to = static_cast<int>(random() % n);
            edge.weight = static_cast<std::int64_t>(random() % 11);
            if (trial % 4 == 0 && random() % 2 == 0)
            {
                edge.weight = kMax - edge.weight;
            }
        }
        std::vector<int> terminals(random() % (n + 2));
        for (auto &terminal : terminals)
        {
            terminal = static_cast<int>(random() % n);
        }

        const auto expected = brute_force(n, edges, terminals);
        if (expected.connected && !expected.cost)
        {
            expect_error<std::overflow_error>(
                [&]()
                {
                    sirkit::minimum_steiner_tree(n, edges, terminals);
                });
        }
        else
        {
            const auto actual = sirkit::minimum_steiner_tree(n, edges, terminals);
            assert(actual.has_value() == expected.cost.has_value());
            if (actual)
            {
                assert(actual->cost == *expected.cost);
                check_tree(n, edges, terminals, *actual);
            }
        }
    }

    expect_error<std::length_error>([]()
                                    {
                                        sirkit::minimum_steiner_tree(-1, {}, {});
                                    });
    constexpr int kMaskBits = std::numeric_limits<std::size_t>::digits;
    std::vector<int> many(kMaskBits + 1);
    std::iota(many.begin(), many.end(), 0);
    expect_error<std::length_error>([&]()
                                    {
                                        sirkit::minimum_steiner_tree(kMaskBits + 1, {}, many);
                                    });
    many.pop_back();
    expect_error<std::length_error>([&]()
                                    {
                                        sirkit::minimum_steiner_tree(kMaskBits, {}, many);
                                    });

#ifndef SKIP_RUNTIME_CHECKS
    for (const int bad : {-1, 3})
    {
        expect_error<std::out_of_range>(
            [&]()
            {
                sirkit::minimum_steiner_tree(3, {}, {&bad, 1});
            });
    }
    for (const auto bad : {sirkit::SteinerTreeEdge{-1, 0, 0}, {3, 0, 0}, {0, -1, 0}, {0, 3, 0}})
    {
        expect_error<std::out_of_range>(
            [&]()
            {
                sirkit::minimum_steiner_tree(3, {&bad, 1}, endpoints);
            });
    }
    const sirkit::SteinerTreeEdge negative{0, 2, -1};
    expect_error<std::invalid_argument>(
        [&]()
        {
            sirkit::minimum_steiner_tree(3, {&negative, 1}, endpoints);
        });
#endif
}
