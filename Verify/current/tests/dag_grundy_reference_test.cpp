// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/DAG_Grundy.cpp"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <set>
#include <vector>

namespace
{
int Grundy(const sirkit::Graph &moves, int vertex, std::vector<int> &cache)
{
    if (cache[vertex] >= 0)
    {
        return cache[vertex];
    }
    std::set<int> successors;
    for (const int next : moves[vertex])
    {
        successors.insert(Grundy(moves, next, cache));
    }
    int answer = 0;
    for (const int value : successors)
    {
        if (value != answer)
        {
            break;
        }
        ++answer;
    }
    return cache[vertex] = answer;
}

bool first_wins(const sirkit::Graph &moves, int first, int second, std::vector<std::vector<int>> &cache)
{
    int &answer = cache[first][second];
    if (answer == -1)
    {
        answer = 0;
        for (const int next : moves[first])
        {
            answer |= !first_wins(moves, next, second, cache);
        }
        for (const int next : moves[second])
        {
            answer |= !first_wins(moves, first, next, cache);
        }
    }
    return answer;
}

void check(const sirkit::Graph &moves)
{
    const int n = static_cast<int>(moves.size());
    auto actual = sirkit::dag_grundy_numbers(moves);
    assert(actual && actual->size() == moves.size());
    std::vector<int> expected(n, -1);
    for (int u = 0; u < n; ++u)
    {
        assert((*actual)[u] == Grundy(moves, u, expected));
    }
    std::vector games(n, std::vector<int>(n, -1));
    for (int u = 0; u < n; ++u)
    {
        for (int v = 0; v < n; ++v)
        {
            assert(first_wins(moves, u, v, games) == (((*actual)[u] ^ (*actual)[v]) != 0));
        }
    }
}
} // namespace

int main()
{
    check({});
    check({{}, {}, {}, {}});
    check({{1, 2, 2}, {}, {1}});
    assert(!sirkit::dag_grundy_numbers({{0}}));
    assert(!sirkit::dag_grundy_numbers({{1}, {2}, {1}, {}}));

    std::mt19937 rng(1757641);
    for (int trial = 0; trial < 1100; ++trial)
    {
        const int n = 1 + rng() % 10;
        std::vector<int> order(n);
        std::iota(order.begin(), order.end(), 0);
        std::ranges::shuffle(order, rng);
        sirkit::Graph moves(n);
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < i; ++j)
            {
                if (rng() % 4 == 0)
                {
                    moves[order[i]].push_back(order[j]);
                    if (rng() % 3 == 0)
                    {
                        moves[order[i]].push_back(order[j]);
                    }
                }
            }
        }
        check(moves);
    }

#ifndef SKIP_RUNTIME_CHECKS
    int rejected = 0;
    for (const int invalid : {-1, 2})
    {
        try
        {
            (void)sirkit::dag_grundy_numbers({{invalid}, {}});
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    assert(rejected == 2);
#endif
}
