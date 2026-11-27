// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_DOMINATOR_TREE_TEST_SOURCE
#define SIRKIT_DOMINATOR_TREE_TEST_SOURCE "../../../Graph/Dominator_Tree.cpp"
#endif
#include SIRKIT_DOMINATOR_TREE_TEST_SOURCE
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
std::vector<bool> Reachable(const std::vector<std::vector<int>> &graph, int root, int blocked)
{
    std::vector<bool> reached(graph.size());
    if (root == blocked)
    {
        return reached;
    }
    std::vector<int> queue{root};
    reached[root] = true;
    for (std::size_t i = 0; i < queue.size(); ++i)
    {
        for (int to : graph[queue[i]])
        {
            if (to != blocked && !reached[to])
            {
                reached[to] = true;
                queue.push_back(to);
            }
        }
    }
    return reached;
}

std::vector<int> DeletionReference(const std::vector<std::vector<int>> &graph, int root)
{
    const int size = static_cast<int>(graph.size());
    const auto reachable = Reachable(graph, root, -1);
    std::vector<std::vector<int>> strict(size);
    for (int blocked = 0; blocked < size; ++blocked)
    {
        const auto without = Reachable(graph, root, blocked);
        for (int v = 0; v < size; ++v)
        {
            if (v != blocked && reachable[v] && !without[v])
            {
                strict[v].push_back(blocked);
            }
        }
    }
    std::vector<int> result(size, -1);
    result[root] = root;
    for (int v = 0; v < size; ++v)
    {
        for (int candidate : strict[v])
        {
            if (result[v] < 0 || strict[result[v]].size() < strict[candidate].size())
            {
                result[v] = candidate;
            }
        }
    }
    return result;
}
} // namespace

int main()
{
    std::mt19937 random(35481);
    for (int n = 1; n <= 13; ++n)
    {
        for (int repetition = 0; repetition < 500; ++repetition)
        {
            std::vector<std::vector<int>> graph(n);
            const auto density = static_cast<unsigned>(repetition % 20);
            for (auto &neighbors : graph)
            {
                for (int to = 0; to < n; ++to)
                {
                    if (random() % 24 < density)
                    {
                        neighbors.push_back(to);
                    }
                    if (random() % 100 == 0)
                    {
                        neighbors.push_back(to);
                    }
                }
                std::shuffle(neighbors.begin(), neighbors.end(), random);
            }
            const int root = static_cast<int>(random() % n);
            assert(sirkit::dominator_tree(graph, root) == DeletionReference(graph, root));
        }
    }

    constexpr int kChainSize = 200'000;
    std::vector<std::vector<int>> chain(kChainSize);
    for (int i = 1; i < kChainSize; ++i)
    {
        chain[i - 1].push_back(i);
    }
    chain.back().push_back(0);
    const auto dominators = sirkit::dominator_tree(chain, 0);
    assert(dominators.front() == 0);
    for (int i = 1; i < kChainSize; ++i)
    {
        assert(dominators[i] == i - 1);
    }

    bool too_many_vertices = false;
    try
    {
        sirkit::detail::CheckDominatorTreeSize(std::numeric_limits<int>::max());
    }
    catch (const std::length_error &)
    {
        too_many_vertices = true;
    }
    assert(too_many_vertices);
#ifndef SKIP_RUNTIME_CHECKS
    const auto rejects = [](const std::vector<std::vector<int>> &graph, int root)
    {
        try
        {
            sirkit::dominator_tree(graph, root);
        }
        catch (const std::out_of_range &)
        {
            return true;
        }
        return false;
    };
    assert(rejects({}, 0));
    assert(rejects({{}}, -1));
    assert(rejects({{-1}}, 0));
    assert(rejects({{1}}, 0));
#endif
}
