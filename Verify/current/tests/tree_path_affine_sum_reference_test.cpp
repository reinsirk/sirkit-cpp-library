// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Path_Affine_Sum.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Value = std::uint64_t;
using Tree = sirkit::TreePathAffineSum<Value>;
using Affine = Tree::Affine;
using Edge = Tree::Edge;

template<class Exception, class Operation>
void must_throw(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

Value brute_sum(std::span<const Value> values, std::span<const Edge> edges, int root)
{
    std::vector<std::vector<std::pair<int, int>>> adjacency(values.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &edge = edges[i];
        adjacency[edge.u].emplace_back(edge.v, static_cast<int>(i));
        adjacency[edge.v].emplace_back(edge.u, static_cast<int>(i));
    }
    struct Visit
    {
        int node;
        int parent;
        Affine transform;
    };
    std::vector<Visit> queue{{root, -1, {1, 0}}};
    Value answer = 0;
    for (std::size_t i = 0; i < queue.size(); ++i)
    {
        const auto [node, parent, transform] = queue[i];
        answer += transform.mul * values[node] + transform.add;
        for (const auto &[to, id] : adjacency[node])
        {
            if (to != parent)
            {
                const auto &edge = edges[id];
                const auto from_child = node == edge.u ? edge.toward_u : edge.toward_v;
                queue.push_back({to, node, {transform.mul * from_child.mul,
                                            transform.mul * from_child.add + transform.add}});
            }
        }
    }
    return answer;
}

void check_exceptions()
{
    const Tree empty({}, {});
    assert(empty.size() == 0 && empty.all_prod() == 0);
    must_throw<std::length_error>([] {
        const auto limit = static_cast<std::size_t>(std::numeric_limits<int>::max() / 5);
        sirkit::detail::check_tree_path_affine_capacity(limit + 1);
    });
#ifndef SKIP_RUNTIME_CHECKS
    must_throw<std::out_of_range>([&] { static_cast<void>(empty.sum_from(0)); });
    const std::array<Value, 2> values{1, 2};
    must_throw<std::invalid_argument>([&] { const Tree tree(values, {}); });
    for (const auto &[u, v] : {std::pair{-1, 1}, std::pair{2, 1}, std::pair{0, -1},
                                std::pair{0, 2}, std::pair{0, 0}})
    {
        const std::array edges{Edge{u, v, {}, {}}};
        must_throw<std::invalid_argument>([&] { const Tree tree(values, edges); });
    }
    const std::array<Value, 4> larger{1, 2, 3, 4};
    const std::array cycle{Edge{0, 1, {}, {}}, Edge{1, 2, {}, {}}, Edge{2, 0, {}, {}}};
    const std::array disconnected{Edge{1, 2, {}, {}}, Edge{2, 3, {}, {}}, Edge{3, 1, {}, {}}};
    must_throw<std::invalid_argument>([&] { const Tree tree(larger, cycle); });
    must_throw<std::invalid_argument>([&] { const Tree tree(larger, disconnected); });
    const std::array edges{Edge{0, 1, {}, {}}};
    Tree tree(values, edges);
    must_throw<std::out_of_range>([&] { tree.set_vertex(-1, 0); });
    must_throw<std::out_of_range>([&] { tree.set_vertex(2, 0); });
    must_throw<std::out_of_range>([&] { tree.set_edge(-1, {}); });
    must_throw<std::out_of_range>([&] { tree.set_edge(1, {}); });
    assert(tree.size() == 2 && tree.all_prod() == 3);
#endif
}

void check_copy()
{
    const std::array<Value, 2> values{7, 10};
    const std::array edges{Edge{1, 0, {3, 4}, {2, 6}}};
    Tree first(values, edges);
    Tree copy = first;
    copy.set_vertex(1, 20);
    assert(first.all_prod() == 33 && copy.all_prod() == 53);
    Tree moved = std::move(copy);
    assert(copy.size() == 0 && copy.all_prod() == 0);
    assert(moved.sum_from(1) == 45);
    copy = moved;
    moved = std::move(first);
    assert(first.size() == 0 && moved.all_prod() == 33 && copy.all_prod() == 53);
}

void check_random()
{
    std::mt19937_64 random(20261128);
    const auto map = [&] { return Affine{random() % 5, random()}; };
    for (int n = 1; n <= 50; ++n)
    {
        for (int shape = 0; shape < 4; ++shape)
        {
            std::vector<Value> values(n);
            for (auto &value : values)
            {
                value = random();
            }
            std::vector<Edge> edges;
            for (int v = 1; v < n; ++v)
            {
                int p = static_cast<int>(random() % v);
                if (shape == 0)
                {
                    p = v - 1;
                }
                else if (shape == 1)
                {
                    p = 0;
                }
                else if (shape == 2)
                {
                    p = (v - 1) / 2;
                }
                if ((random() & 1) == 0)
                {
                    edges.push_back({v, p, map(), map()});
                }
                else
                {
                    edges.push_back({p, v, map(), map()});
                }
            }
            std::shuffle(edges.begin(), edges.end(), random);
            Tree tree(values, edges);
            assert(tree.size() == n);
            for (int query = 0; query < 70; ++query)
            {
                const int root = static_cast<int>(random() % n);
                const int v = static_cast<int>(random() % n);
                if (edges.empty() || query % 3 == 0)
                {
                    values[v] = random();
                    tree.set_vertex(v, values[v]);
                }
                else
                {
                    const int id = static_cast<int>(random() % edges.size());
                    auto &edge = edges[id];
                    edge.toward_u = map();
                    if (query % 3 == 1)
                    {
                        edge.toward_v = edge.toward_u;
                        tree.set_edge(id, edge.toward_u);
                    }
                    else
                    {
                        edge.toward_v = map();
                        tree.set_edge(id, edge.toward_u, edge.toward_v);
                    }
                }
                assert(tree.all_prod() == brute_sum(values, edges, 0));
                assert(tree.sum_from(root) == brute_sum(values, edges, root));
            }
        }
    }
}

void check_large(bool star)
{
    constexpr int kSize = 200000;
    constexpr int kMiddle = kSize / 2;
    std::vector<Value> values(kSize, 1);
    std::vector<Edge> edges;
    edges.reserve(kSize - 1);
    for (int v = 1; v < kSize; ++v)
    {
        edges.push_back({v, star ? 0 : v - 1, {1, 1}, {1, 1}});
    }
    Tree tree(values, edges);
    const auto triangle = [](Value d) { return d * (d + 1) / 2; };
    const Value top = star ? kSize - 1 : triangle(kSize - 1);
    const Value middle = star ? 2 * kSize - 3 : triangle(kMiddle) + triangle(kSize - 1 - kMiddle);
    assert(tree.all_prod() == kSize + top);
    assert(tree.sum_from(kMiddle) == kSize + middle);
    tree.set_vertex(kMiddle, 4);
    assert(tree.all_prod() == kSize + top + 3);
    assert(tree.sum_from(kMiddle) == kSize + middle + 3);
}
}

int main()
{
    check_exceptions();
    check_copy();
    check_random();
    check_large(false);
    check_large(true);
}
