// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Contour.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using sirkit::ContourAddVertexGet;
using sirkit::TreeContourEdge;
using sirkit::TreeContourIndex;
using sirkit::VertexAddContourSum;
using Value = std::int64_t;
using Edge = std::pair<int, int>;

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

std::vector<std::vector<Value>> tree_distances(int n, const std::vector<TreeContourEdge> &edges)
{
    std::vector<std::vector<std::pair<int, Value>>> graph(n);
    for (const auto &[u, v, length] : edges)
    {
        graph[u].emplace_back(v, length);
        graph[v].emplace_back(u, length);
    }
    std::vector<std::vector<Value>> distance(n, std::vector<Value>(n, -1));
    for (int start = 0; start < n; ++start)
    {
        distance[start][start] = 0;
        std::vector<int> queue{start};
        for (std::size_t i = 0; i < queue.size(); ++i)
        {
            const int u = queue[i];
            for (const auto &[v, w] : graph[u])
            {
                if (distance[start][v] < 0)
                {
                    distance[start][v] = distance[start][u] + w;
                    queue.push_back(v);
                }
            }
        }
    }
    return distance;
}

void check(int n, const std::vector<TreeContourEdge> &edges, bool weighted, std::mt19937 &random)
{
    const auto all_distances = tree_distances(n, edges);
    std::vector<Edge> endpoints;
    for (const auto &edge : edges)
    {
        endpoints.emplace_back(edge.from, edge.to);
    }
    const auto index = weighted ? TreeContourIndex::weighted(n, edges) : TreeContourIndex(n, endpoints);
    assert(index.size() == n);
    std::vector<Value> plain_sum(n);
    for (auto &value : plain_sum)
    {
        value = static_cast<Value>(random() % 31) - 15;
    }
    auto plain_range = plain_sum;
    VertexAddContourSum<> sums(index, plain_sum);
    ContourAddVertexGet<> ranges(index, plain_range);
    for (int query = 0; query < 185; ++query)
    {
        const int v = static_cast<int>(random() % n), u = static_cast<int>(random() % n);
        const Value left = static_cast<Value>(random() % (3 * n + 15)) - n / 2 - 5;
        const Value right = static_cast<Value>(random() % (3 * n + 15)) - n / 2 - 5;
        const Value point_delta = static_cast<Value>(random() % 61) - 30;
        const Value range_delta = static_cast<Value>(random() % 61) - 30;
        sums.add(u, point_delta);
        plain_sum[u] += point_delta;
        ranges.add_range(v, left, right, range_delta);

        Value expected = 0;
        for (int i = 0; i < n; ++i)
        {
            const Value d = all_distances[v][i];
            if (left <= d && d < right)
            {
                expected += plain_sum[i];
                plain_range[i] += range_delta;
            }
        }
        assert(sums.sum(v, left, right) == expected && ranges.get(u) == plain_range[u]);
    }
    for (int v = 0; v < n; ++v)
    {
        assert(ranges.get(v) == plain_range[v]);
    }
}

struct Residue
{
    int value = 0;

    Residue &operator+=(const Residue &other)
    {
        value = (value + other.value) % 7;
        return *this;
    }

    Residue &operator-=(const Residue &other)
    {
        value = (value + 7 - other.value) % 7;
        return *this;
    }
};
} // namespace

int main()
{
    const TreeContourIndex empty(0, {});
    assert(TreeContourIndex::weighted(0, {}).size() == 0);
    VertexAddContourSum<> empty_sums(empty);
    ContourAddVertexGet<> empty_ranges(empty);
    assert(empty.size() == 0);
    TreeContourIndex one(1, {});
    const auto moved = std::move(one);
    assert(one.size() == 0 && moved.size() == 1);
    VertexAddContourSum<> from_moved(one);
    ContourAddVertexGet<> dual_from_moved(one);
    VertexAddContourSum<> point(moved);
    ContourAddVertexGet<> dual(moved);
    point.add(0, -7);
    dual.add_range(0, -1, 1, -9);
    dual.add_range(0, 0, 0, 7);
    assert(point.sum(0, -1, 1) == -7 && point.sum(0, 1, 2) == 0);
    assert(point.sum(0, -3, -1) == 0);
    assert(dual.get(0) == -9);

    constexpr auto kMax = std::numeric_limits<Value>::max();
    const std::array wide_edges{TreeContourEdge{0, 1, kMax - 3}, TreeContourEdge{1, 2, 9},
                                TreeContourEdge{2, 3, 0}, TreeContourEdge{3, 4, 2}};
    const auto wide = TreeContourIndex::weighted(5, wide_edges);
    constexpr std::array<Value, 5> values{1, 2, 4, 8, 16};
    VertexAddContourSum<> sums(wide, values);
    ContourAddVertexGet<> ranges(wide, values);
    assert(sums.sum(0, kMax - 3, kMax) == 2);
    assert(sums.sum(3, std::numeric_limits<Value>::min(), 1) == 12);
    assert(sums.sum(4, kMax - 2, kMax) == 0);
    ranges.add_range(0, kMax - 3, kMax, 17);
    ranges.add_range(3, -2, 1, -4);
    for (int v = 0; v < 5; ++v)
    {
        assert((ranges.get(v) == std::array<Value, 5>{1, 19, 0, 4, 16}[v]));
    }

    const std::array e{Edge{0, 1}, Edge{1, 2}, Edge{2, 3}, Edge{1, 4}};
    const TreeContourIndex index(5, e);
    VertexAddContourSum<Residue> modular_sum(index);
    ContourAddVertexGet<Residue> modular_add(index);
    modular_sum.add(0, {5});
    modular_sum.add(4, {4});
    modular_add.add_range(1, 1, 2, {5});
    modular_add.add_range(1, 1, 4, {4});
    assert(modular_sum.sum(1, 1, 2).value == 2);
    assert(modular_sum.sum(0, -2, 5).value == 2);
    assert(modular_sum.sum(0, -2, 1).value == 5);
    assert(modular_sum.sum(0, 3, 4).value == 0);
    assert(modular_sum.sum(0, 2, 2).value == 0);
    assert(modular_sum.sum(0, -2, -1).value == 0);
    assert(modular_add.get(0).value == 2 && modular_add.get(3).value == 4);
    modular_add.add_range(0, -2, 5, {1});
    modular_add.add_range(0, -2, 1, {2});
    modular_add.add_range(0, 3, 4, {2});
    modular_add.add_range(0, 2, 2, {3});
    modular_add.add_range(0, -2, -1, {3});
    assert(modular_add.get(0).value == 5 && modular_add.get(3).value == 0);
    const std::array<Residue, 5> modular_initial{{{1}, {2}, {3}, {4}, {5}}};
    const VertexAddContourSum<Residue> modular_from_values(index, modular_initial);
    const ContourAddVertexGet<Residue> modular_range_from_values(index, modular_initial);
    assert(modular_from_values.sum(0, 1, 4).value == 0 && modular_range_from_values.get(2).value == 3);

    std::mt19937 random(822395);
    for (int trial = 0; trial < 500; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 26);
        const bool weighted = (trial & 1) == 0;
        std::vector<TreeContourEdge> edges;
        for (int v = 1; v < n; ++v)
        {
            edges.push_back({v, static_cast<int>(random() % v), weighted ? static_cast<Value>(random() % 10) : 1});
        }
        std::shuffle(edges.begin(), edges.end(), random);
        check(n, edges, weighted, random);
    }

    expect_error<std::length_error>([]
                                    {
                                        static_cast<void>(TreeContourIndex(-1, {}));
                                    });
    expect_error<std::length_error>([]
                                    {
                                        static_cast<void>(TreeContourIndex(std::numeric_limits<int>::max(), {}));
                                    });
    expect_error<std::invalid_argument>([]
                                        {
                                            static_cast<void>(TreeContourIndex(2, {}));
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            static_cast<void>(VertexAddContourSum<>(moved, std::array<Value, 2>{}));
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            static_cast<void>(ContourAddVertexGet<>(moved, std::array<Value, 2>{}));
                                        });
#ifndef SKIP_RUNTIME_CHECKS
    for (Edge edge : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            static_cast<void>(TreeContourIndex(2, {&edge, 1}));
                                        });
    }
    const TreeContourEdge negative{0, 1, -1};
    expect_error<std::invalid_argument>([&]
                                        {
                                            static_cast<void>(TreeContourIndex::weighted(2, {&negative, 1}));
                                        });
    const std::array disconnected{Edge{0, 1}, Edge{1, 0}};
    expect_error<std::invalid_argument>([&]
                                        {
                                            static_cast<void>(TreeContourIndex(3, disconnected));
                                        });
    expect_error<std::out_of_range>([&]
                                    {
                                        static_cast<void>(sums.sum(-1, 0, 1));
                                    });
    expect_error<std::out_of_range>([&]
                                    {
                                        static_cast<void>(ranges.get(5));
                                    });
    expect_error<std::out_of_range>([&]
                                    {
                                        empty_sums.add(0, 1);
                                    });
    expect_error<std::out_of_range>([&]
                                    {
                                        empty_ranges.add_range(0, 0, 0, 1);
                                    });
#endif
}
