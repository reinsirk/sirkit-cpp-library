// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Static_Top_Tree.cpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Mint = std::uint64_t;
constexpr Mint kMod = 998244353;

struct HashDP
{
    using Vertex = Mint;
    using Point = Mint;
    using Path = std::pair<Mint, Mint>;

    int *calls;

    Path vertex(Vertex value, const Point *children) const noexcept
    {
        ++*calls;
        return {children ? *children : 1, value};
    }

    Point project(const Path &path) const noexcept
    {
        ++*calls;
        return path.second;
    }

    Point rake(Point a, Point b) const noexcept
    {
        ++*calls;
        return a * b % kMod;
    }

    Path compress(const Path &upper, const Path &lower) const noexcept
    {
        ++*calls;
        return {upper.first * lower.first % kMod,
                (upper.first * lower.second + upper.second) % kMod};
    }
};

struct BoolDP
{
    using Vertex = bool;
    using Path = bool;
    using Point = bool;

    bool vertex(bool value, const bool *other) const noexcept
    {
        return value || (other && *other);
    }

    bool project(bool path) const noexcept
    {
        return path;
    }

    bool rake(bool left, bool right) const noexcept
    {
        return left || right;
    }

    bool compress(bool upper, bool lower) const noexcept
    {
        return upper || lower;
    }
};

using Tree = sirkit::StaticTopTree<HashDP>;
using Edge = Tree::Edge;

class HashReference
{
public:
    HashReference(int n, int root, const std::vector<Edge> &edges)
        : parent_(n, -1), order_{root}, leaves_(n, true)
    {
        std::vector<std::vector<int>> graph(n);
        for (const auto &[u, v] : edges)
        {
            graph[u].push_back(v);
            graph[v].push_back(u);
        }
        for (std::size_t i = 0; i < order_.size(); ++i)
        {
            for (const int child : graph[order_[i]])
            {
                if (child == parent_[order_[i]])
                {
                    continue;
                }
                leaves_[order_[i]] = false;
                parent_[child] = order_[i];
                order_.push_back(child);
            }
        }
    }

    Mint calculate(const std::vector<Mint> &values) const
    {
        std::vector<Mint> product(values.size(), 1);
        for (auto at = order_.rbegin(); at != order_.rend(); ++at)
        {
            const int v = *at;
            const Mint result = (values[v] + (leaves_[v] ? 0 : product[v])) % kMod;
            if (parent_[v] == -1)
            {
                return result;
            }
            product[parent_[v]] = product[parent_[v]] * result % kMod;
        }
        std::abort();
    }

private:
    std::vector<int> parent_;
    std::vector<int> order_;
    std::vector<bool> leaves_;
};

template<class Exception, class Call>
void expect_throw(Call call)
{
    bool caught = false;
    try
    {
        call();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

void check_invalid_and_lifecycle()
{
    int calls = 0;
    const HashDP operations{&calls};
    const std::array<Mint, 1> leaf{7};
    const std::array<Mint, 4> values{0, 1, 2, 3};
    const std::array<Edge, 3> edges{Edge{0, 1}, Edge{0, 2}, Edge{2, 3}};
    Tree single(leaf, {}, 0, operations);
    assert(single.size() == 1 && single.all_prod().second == 7);
    single.set(0, 0);
    assert(single.all_prod().second == 0);
    Tree tree(values, edges, 2, operations);
    assert(tree.size() == 4 && tree.all_prod().second == 5);
    single = tree;
    tree.set(3, 12);
    assert(single.all_prod().second == 5 && tree.all_prod().second == 14);
    auto moved = std::move(single);
    tree = std::move(moved);
    assert(tree.all_prod().second == 5);
    single = tree;
    assert(single.all_prod().second == 5);

    expect_throw<std::length_error>([] {
        sirkit::detail::check_static_top_tree_capacity(
            static_cast<std::size_t>(std::numeric_limits<int>::max() / 3) + 1);
    });
#ifndef SKIP_RUNTIME_CHECKS
    expect_throw<std::logic_error>([&] { static_cast<void>(moved.all_prod()); });
    expect_throw<std::out_of_range>([&] { tree.set(-1, 2); });
    expect_throw<std::out_of_range>([&] { tree.set(4, 2); });
    expect_throw<std::out_of_range>([&] { const Tree invalid({}, {}, 0, operations); });
    expect_throw<std::out_of_range>([&] { const Tree invalid(values, edges, -1, operations); });
    expect_throw<std::out_of_range>([&] { const Tree invalid(values, edges, 4, operations); });
    expect_throw<std::invalid_argument>([&] { const Tree invalid(values, {}, 0, operations); });
    for (const Edge &bad : std::array{Edge{-1, 0}, Edge{4, 0}, Edge{0, -1}, Edge{0, 4}, Edge{0, 0}})
    {
        const std::array invalid_edges{bad, Edge{0, 2}, Edge{2, 3}};
        expect_throw<std::invalid_argument>([&] { const Tree invalid(values, invalid_edges, 0, operations); });
    }
    const std::array repeated{Edge{0, 1}, Edge{1, 0}, Edge{2, 3}};
    expect_throw<std::invalid_argument>([&] { const Tree invalid(values, repeated, 0, operations); });
    const std::array disconnected{Edge{1, 2}, Edge{2, 3}, Edge{3, 1}};
    expect_throw<std::invalid_argument>([&] { const Tree invalid(values, disconnected, 0, operations); });
#endif

    const std::array<bool, 4> bits{false, false, false, false};
    const std::array star{Edge{0, 1}, Edge{0, 2}, Edge{0, 3}};
    sirkit::StaticTopTree<BoolDP> boolean(bits, star);
    assert(!boolean.all_prod());
    boolean.set(2, true);
    assert(boolean.all_prod());
    boolean.set(2, false);
    assert(!boolean.all_prod());
}

void test_small_random()
{
    std::mt19937_64 rng(901827);
    for (int trial = 0; trial < 1000; ++trial)
    {
        const int n = 1 + rng() % 70;
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            Edge edge{static_cast<int>(rng() % v), v};
            if (rng() % 2)
            {
                std::swap(edge.first, edge.second);
            }
            edges.push_back(edge);
        }
        std::shuffle(edges.begin(), edges.end(), rng);
        const int root = rng() % n;
        std::vector<Mint> values(n);
        for (auto &value : values)
        {
            value = rng() % kMod;
        }
        int calls = 0;
        Tree tree(values, edges, root, HashDP{&calls});
        HashReference reference(n, root, edges);
        assert(calls < 3 * n && tree.all_prod().second == reference.calculate(values));
        for (int step = 0; step < 40; ++step)
        {
            const int v = rng() % n;
            values[v] = step % 2 == 0 ? 0 : rng() % kMod;
            calls = 0;
            tree.set(v, values[v]);
            assert(calls <= 9 * static_cast<int>(std::bit_width(static_cast<unsigned>(n))));
            assert(tree.all_prod().second == reference.calculate(values));
        }
    }
}

void test_large_shapes()
{
    constexpr int kSize = 200000;
    std::mt19937_64 rng(2718);
    for (int shape = 0; shape < 3; ++shape)
    {
        std::vector<Edge> edges;
        edges.reserve(kSize - 1);
        for (int i = 1; i < kSize; ++i)
        {
            const int parent = shape == 0 ? i - 1 : shape == 1 ? 0 : (i % 2 == 0 ? i - 2 : i - 1);
            edges.emplace_back(parent, i);
        }
        std::vector<Mint> values(kSize, 1);
        const int root = shape == 1 ? 0 : kSize - 1;
        int calls = 0;
        Tree tree(values, edges, root, HashDP{&calls});
        const HashReference reference(kSize, root, edges);
        assert(calls < 3 * kSize && tree.all_prod().second == reference.calculate(values));
        for (int step = 0; step < 18; ++step)
        {
            const int v = rng() % kSize;
            values[v] = step + 2;
            calls = 0;
            tree.set(v, values[v]);
            assert(calls <= 9 * static_cast<int>(std::bit_width(static_cast<unsigned>(kSize))));
            assert(tree.all_prod().second == reference.calculate(values));
        }
    }
}
}

int main()
{
    check_invalid_and_lifecycle();
    test_small_random();
    test_large_shapes();
}
