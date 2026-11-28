// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Offline_DAG_Reachability.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <numeric>
#include <queue>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Pair = std::pair<int, int>;

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

void check_limits()
{
    assert(sirkit::offline_dag_reachability(0, {}, {}).empty());
    const std::array singleton{Pair{0, 0}};
    assert(sirkit::offline_dag_reachability(1, {}, singleton).front());
    const std::array three_queries{Pair{1, 0}, Pair{0, 0}, Pair{2, 1}};
    assert(sirkit::offline_dag_reachability(3, {}, three_queries) == (std::vector<bool>{false, true, false}));
    must_throw<std::length_error>([] { static_cast<void>(sirkit::offline_dag_reachability(-1, {}, {})); });
#ifndef SKIP_RUNTIME_CHECKS
    must_throw<std::invalid_argument>([&] { static_cast<void>(sirkit::offline_dag_reachability(1, singleton, {})); });
    const std::array cycle{Pair{0, 1}, Pair{1, 2}, Pair{2, 0}};
    must_throw<std::invalid_argument>([&] { static_cast<void>(sirkit::offline_dag_reachability(3, cycle, {})); });
    for (const auto &pair : {Pair{-1, 0}, Pair{0, -1}, Pair{2, 0}, Pair{0, 2}})
    {
        const std::span input(&pair, 1);
        must_throw<std::out_of_range>([&] { static_cast<void>(sirkit::offline_dag_reachability(2, input, {})); });
        must_throw<std::out_of_range>([&] { static_cast<void>(sirkit::offline_dag_reachability(2, {}, input)); });
    }
#endif
}

std::vector<bool> brute(const std::vector<std::vector<int>> &adjacency, std::span<const Pair> queries)
{
    std::vector<bool> answer;
    for (const auto &[from, to] : queries)
    {
        std::vector<bool> seen(adjacency.size());
        std::queue<int> pending;
        pending.push(from);
        seen[from] = true;
        while (!pending.empty())
        {
            const int v = pending.front();
            pending.pop();
            for (const int u : adjacency[v])
            {
                if (!seen[u])
                {
                    seen[u] = true;
                    pending.push(u);
                }
            }
        }
        answer.push_back(seen[to]);
    }
    return answer;
}

void check_random()
{
    std::mt19937 rng(59);
    for (int test = 0; test < 260; ++test)
    {
        const int n = 1 + static_cast<int>(rng() % 60);
        std::vector<int> order(n);
        std::iota(order.begin(), order.end(), 0);
        if (test % 2)
        {
            std::shuffle(order.begin(), order.end(), rng);
        }
        std::vector<std::vector<int>> adjacency(n);
        std::vector<Pair> edges, queries;
        for (int u = 0; u < n; ++u)
        {
            for (int v = u + 1; v < n; ++v)
            {
                if (rng() % 100 < static_cast<unsigned>(test % 95))
                {
                    edges.emplace_back(order[u], order[v]);
                    adjacency[order[u]].push_back(order[v]);
                    if (rng() % 5 == 0)
                    {
                        edges.emplace_back(order[u], order[v]);
                        adjacency[order[u]].push_back(order[v]);
                    }
                }
            }
        }
        for (int q = 0; q < 90; ++q)
        {
            const int from = test % 3 == 0 ? order.front() : static_cast<int>(rng() % n);
            const int to = test % 3 == 1 ? order.back() : static_cast<int>(rng() % n);
            queries.emplace_back(from, to);
        }
        assert(sirkit::offline_dag_reachability(n, edges, queries) == brute(adjacency, queries));
    }
}

void check_word_boundaries()
{
    constexpr int kVertices = 264;
    std::vector<std::vector<int>> adjacency(kVertices);
    std::vector<Pair> edges, queries;
    for (int v = 0; v + 2 < kVertices; ++v)
    {
        edges.emplace_back(v, v + 2);
        adjacency[v].push_back(v + 2);
    }
    for (int v = 0; v < 128; ++v)
    {
        queries.emplace_back(v, 256 - v);
    }
    assert(sirkit::offline_dag_reachability(kVertices, edges, queries) == brute(adjacency, queries));
    queries.emplace_back(130, 261);
    assert(sirkit::offline_dag_reachability(kVertices, edges, queries) == brute(adjacency, queries));
}
}

int main()
{
    check_limits();
    check_random();
    check_word_boundaries();
}
