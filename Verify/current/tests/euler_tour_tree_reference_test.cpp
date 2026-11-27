// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Euler_Tour_Tree.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Value = std::int64_t;
using Forest = sirkit::EulerTourTree<>;

struct Reference
{
    std::vector<std::vector<bool>> edges;
    std::vector<Value> values;

    std::vector<int> side(int v, int parent) const
    {
        std::vector<bool> visited(edges.size());
        std::vector<int> seen{v};
        visited[v] = true;
        for (std::size_t i = 0; i < seen.size(); ++i)
        {
            const int u = seen[i];
            for (int to = 0; to < static_cast<int>(edges.size()); ++to)
            {
                if (edges[u][to] && !visited[to] && (u != v || to != parent))
                {
                    visited[to] = true;
                    seen.push_back(to);
                }
            }
        }
        return seen;
    }

    bool connected(int a, int b) const
    {
        const auto vertices = side(a, -1);
        return std::find(vertices.begin(), vertices.end(), b) != vertices.end();
    }

    bool link(int a, int b)
    {
        if (connected(a, b))
        {
            return false;
        }
        edges[a][b] = edges[b][a] = true;
        return true;
    }

    bool cut(int a, int b)
    {
        if (!edges[a][b])
        {
            return false;
        }
        edges[a][b] = edges[b][a] = false;
        return true;
    }

    void add(int v, int parent, Value delta)
    {
        for (int u : side(v, parent))
        {
            values[u] += delta;
        }
    }

    Value sum(int v, int parent) const
    {
        Value result = 0;
        for (int u : side(v, parent))
        {
            result += values[u];
        }
        return result;
    }
};

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

void simulate(int n, std::mt19937 &random)
{
    std::vector<Value> initial(n);
    for (auto &value : initial)
    {
        value = static_cast<Value>(random() % 201) - 100;
    }
    Forest work(n, initial);
    Reference simple{std::vector<std::vector<bool>>(n, std::vector<bool>(n)), initial};
    int components = n;
    for (int q = 0; q < 620; ++q)
    {
        const int v = static_cast<int>(random() % n), p = static_cast<int>(random() % n);
        const auto delta = static_cast<Value>(random() % 201) - 100;
        switch (random() % 10)
        {
        case 0:
        case 1:
        {
            const bool linked = simple.link(v, p);
            assert(work.link(v, p) == linked);
            components -= linked;
            break;
        }
        case 2:
        case 3:
        {
            const bool cut = simple.cut(v, p);
            assert(work.cut(v, p) == cut);
            components += cut;
            break;
        }
        case 4:
            work.add(v, delta);
            simple.values[v] += delta;
            break;
        case 5:
            work.component_add(v, delta);
            simple.add(v, -1, delta);
            break;
        case 6:
            assert(work.component_sum(v) == simple.sum(v, -1));
            break;
        case 7:
            assert(work.subtree_add(v, p, delta) == simple.edges[v][p]);
            if (simple.edges[v][p])
            {
                simple.add(v, p, delta);
            }
            break;
        case 8:
            if (simple.edges[v][p])
            {
                assert(work.subtree_sum(v, p) == simple.sum(v, p));
                assert(work.subtree_sum(p, v) == simple.sum(p, v));
            }
            else
            {
                assert(!work.subtree_sum(v, p));
            }
            break;
        default:
            assert(work.connected(v, p) == simple.connected(v, p));
            break;
        }
        assert(work.get(v) == simple.values[v]);
        assert(work.size() == n && work.component_count() == components);
    }
    for (int v = 0; v < n; ++v)
    {
        assert(work.get(v) == simple.values[v]);
        for (int p = 0; p < n; ++p)
        {
            assert(work.connected(v, p) == simple.connected(v, p));
        }
    }
}
} // namespace

int main()
{
    Forest empty(0);
    assert(empty.size() == 0 && empty.component_count() == 0);
    Forest work(1);
    assert(work.connected(0, 0));
    assert(!work.link(0, 0) && !work.cut(0, 0));
    assert(!work.subtree_add(0, 0, 3) && !work.subtree_sum(0, 0));
    work.add(0, -10);
    assert(work.component_sum(0) == -10);
    work.component_add(0, 20);
    assert(work.get(0) == 10);

    Forest chain(4, std::array<Value, 4>{1, 10, 100, 1000});
    assert(chain.link(3, 2) && chain.link(1, 0) && chain.link(2, 1));
    assert(!chain.link(0, 3));
    assert(chain.subtree_sum(2, 1) == 1100 && chain.subtree_sum(1, 2) == 11);
    assert(chain.subtree_add(2, 1, 17));
    chain.component_add(3, -3);
    Forest copy = chain;
    assert(copy.cut(1, 2));
    assert(copy.link(0, 3));
    assert(chain.subtree_sum(2, 1) == 1128);
    assert(copy.subtree_sum(0, 3) == 5);
    copy.add(2, 30);
    assert(chain.component_sum(3) == 1133 && copy.component_sum(0) == 1163);
    Forest moved(std::move(copy));
    assert(copy.size() == 0 && copy.component_count() == 0);
    copy = std::move(moved);
    assert(copy.component_sum(0) == 1163);
    assert(moved.size() == 0 && moved.component_count() == 0);
    copy.swap(work);
    assert(copy.get(0) == 10 && work.component_sum(0) == 1163);
    work = chain;
    assert(work.component_sum(0) == 1133 && chain.component_sum(1) == 1133);

    std::mt19937 random(8638805);
    for (int trial = 0; trial < 310; ++trial)
    {
        simulate(1 + static_cast<int>(random() % 28), random);
    }

    expect_error<std::length_error>([]
                                    {
                                        static_cast<void>(Forest(-1));
                                    });
    expect_error<std::length_error>([]
                                    {
                                        static_cast<void>(Forest(std::numeric_limits<int>::max()));
                                    });
    expect_error<std::invalid_argument>([]
                                        {
                                            static_cast<void>(Forest(3, std::array<Value, 2>{}));
                                        });
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::out_of_range>([&]
                                    {
                                        static_cast<void>(empty.get(0));
                                    });
    expect_error<std::out_of_range>([&]
                                    {
                                        static_cast<void>(work.connected(-1, 0));
                                    });
#endif
}
