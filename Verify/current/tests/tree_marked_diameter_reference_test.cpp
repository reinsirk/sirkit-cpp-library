// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Marked_Diameter.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using sirkit::TreeMarkedDiameter;

void check_sample_family()
{
    TreeMarkedDiameter empty(0, {}, true);
    assert(empty.size() == 0 && !empty.diameter());
    TreeMarkedDiameter single(1, {}, true);
    assert(single.marked(0) && single.diameter()->distance == 0);
    assert(single.subtree_diameter(0)->first == 0);
    assert(!single.toggle(0) && !single.farthest(0));
    assert(single.toggle(0) && single.farthest(0)->distance == 0);

    const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
    TreeMarkedDiameter original(3, edges, true);
    TreeMarkedDiameter copied(original);
    copied.set_marked(2, false);
    copied.set_marked(2, false);
    assert(original.marked(2) && !copied.marked(2));
    single = copied;
    copied.toggle(1);
    assert(single.diameter()->distance == 1);
    assert(copied.diameter()->distance == 0);
    TreeMarkedDiameter moved(std::move(single));
    assert(!single.diameter() && moved.diameter()->distance == 1);
    copied = std::move(moved);
    assert(moved.size() == 0 && copied.farthest(2)->distance == 2);
}

void check_random()
{
    std::mt19937 random(12341);
    for (int trial = 0; trial < 65; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 25);
        std::vector<std::pair<int, int>> edges;
        std::vector<int> parent(n, -1);
        std::vector<std::vector<int>> adjacency(n);
        for (int v = 1; v < n; ++v)
        {
            const int p = static_cast<int>(random() % v);
            edges.emplace_back(v, p);
            parent[v] = p;
            adjacency[v].push_back(p);
            adjacency[p].push_back(v);
        }
        std::shuffle(edges.begin(), edges.end(), random);

        std::vector<std::vector<int>> distances(n, std::vector<int>(n, -1));
        for (int start = 0; start < n; ++start)
        {
            auto &d = distances[start];
            std::queue<int> pending;
            d[start] = 0;
            pending.push(start);
            while (!pending.empty())
            {
                const int u = pending.front();
                pending.pop();
                for (const int v : adjacency[u])
                {
                    if (d[v] < 0)
                    {
                        d[v] = d[u] + 1;
                        pending.push(v);
                    }
                }
            }
        }

        const bool initially_marked = trial % 2 == 0;
        TreeMarkedDiameter tree(n, edges, initially_marked);
        std::vector<bool> mask(n, initially_marked);
        for (int step = 0; step < 85; ++step)
        {
            const int v = static_cast<int>(random() % n);
            if (step % 3 == 0)
            {
                mask[v] = !mask[v];
                assert(tree.toggle(v) == mask[v]);
            }
            else
            {
                mask[v] = (random() % 2) == 0;
                tree.set_marked(v, mask[v]);
            }
            assert(tree.marked(v) == mask[v]);

            const int root = static_cast<int>(random() % n);
            std::vector<int> all, subtree;
            for (int s = 0; s < n; ++s)
            {
                if (!mask[s])
                {
                    continue;
                }
                all.push_back(s);
                for (int t = s; t != -1; t = parent[t])
                {
                    if (t == root)
                    {
                        subtree.push_back(s);
                        break;
                    }
                }
            }
            const auto check = [&](auto answer, const std::vector<int> &members)
            {
                if (members.empty())
                {
                    assert(!answer);
                    return;
                }
                int maximum = 0;
                for (const int a : members)
                {
                    for (const int b : members)
                    {
                        maximum = std::max(maximum, distances[a][b]);
                    }
                }
                assert(answer && answer->distance == maximum);
                assert(std::binary_search(members.begin(), members.end(), answer->first));
                assert(std::binary_search(members.begin(), members.end(), answer->second));
                assert(distances[answer->first][answer->second] == maximum);
            };
            check(tree.diameter(), all);
            check(tree.subtree_diameter(root), subtree);
            const auto most_remote = tree.farthest(v);
            if (all.empty())
            {
                assert(!most_remote);
            }
            else
            {
                int maximum = 0;
                for (const int a : all)
                {
                    maximum = std::max(maximum, distances[v][a]);
                }
                assert(most_remote && mask[most_remote->vertex]);
                assert(most_remote->distance == maximum && distances[v][most_remote->vertex] == maximum);
            }
        }
    }
}

#ifndef SKIP_RUNTIME_CHECKS
template <class Exception, class Operation>
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

void check_failures()
{
    must_throw<std::invalid_argument>([]
                                      {
                                          TreeMarkedDiameter forest(2, {});
                                      });
    const std::array loop{std::pair{0, 0}};
    must_throw<std::invalid_argument>([&]
                                      {
                                          TreeMarkedDiameter graph(2, loop);
                                      });
    TreeMarkedDiameter tree(1, {});
    must_throw<std::out_of_range>([&]
                                  {
                                      static_cast<void>(tree.farthest(1));
                                  });
    must_throw<std::out_of_range>([&]
                                  {
                                      static_cast<void>(tree.subtree_diameter(-1));
                                  });
    must_throw<std::out_of_range>([&]
                                  {
                                      tree.set_marked(1);
                                  });
}
#endif
}

int main()
{
    check_sample_family();
    check_random();
#ifndef SKIP_RUNTIME_CHECKS
    check_failures();
#endif
}
