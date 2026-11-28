// SPDX-License-Identifier: CC0-1.0
#include "Graph/Subtree_Intervals.cpp"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using sirkit::SubtreeIntervals;

void Expect(bool predicate)
{
    if (!predicate)
    {
        std::abort();
    }
}

template <class Error, class F>
void ExpectThrow(F operation)
{
    try
    {
        operation();
    }
    catch (const Error &)
    {
        return;
    }
    std::abort();
}

bool IsAncestor(int from, int to, const std::vector<int> &parent)
{
    for (; to >= 0; to = parent[to])
    {
        if (to == from)
        {
            return true;
        }
    }
    return false;
}

void RandomForests()
{
    std::mt19937 rng(7257177);
    for (int trial = 0; trial < 260; ++trial)
    {
        const int n = rng() % 60;
        std::vector<int> order(n);
        std::iota(order.begin(), order.end(), 0);
        std::shuffle(order.begin(), order.end(), rng);
        std::vector<int> parent(n, -1);
        for (int k = 1; k < n; ++k)
        {
            if (rng() % 6 != 0)
            {
                parent[order[k]] = order[rng() % k];
            }
        }

        const SubtreeIntervals actual(parent);
        Expect(actual.size() == n);
        std::vector<int> expected;
        const auto visit = [&](auto &&self, int v) -> void
        {
            expected.push_back(v);
            for (int w = 0; w < n; ++w)
            {
                if (parent[w] == v)
                {
                    self(self, w);
                }
            }
        };
        for (int v = 0; v < n; ++v)
        {
            if (parent[v] == -1)
            {
                visit(visit, v);
            }
        }
        Expect(std::ranges::equal(expected, actual.order()));
        for (int v = 0; v < n; ++v)
        {
            const auto [left, right] = actual.range(v);
            Expect(left == actual.index(v) && actual.vertex_at(left) == v);
            for (int u = 0; u < n; ++u)
            {
                const bool ancestor = IsAncestor(v, u, parent);
                const bool interval = left <= actual.index(u) && actual.index(u) < right;
                Expect(ancestor == interval && actual.contains(v, u) == interval);
            }
        }
    }
}

void Boundaries()
{
    const int n = 500000;
    std::vector<int> parent(n);
    std::iota(parent.begin(), parent.end(), 1);
    parent.back() = -1;
    SubtreeIntervals actual(parent);
    Expect(actual.range(n - 1) == (std::pair{0, n}));
    Expect(actual.range(0) == (std::pair{n - 1, n}));
    auto copy = actual;
    const auto moved = std::move(actual);
    Expect(actual.size() == 0 && actual.order().empty());
    Expect(copy.index(2) == n - 3 && moved.index(1) == n - 2);
    actual = copy;
    Expect(actual.contains(n - 1, 0) && !actual.contains(0, n - 1));
    ExpectThrow<std::length_error>([]
                                   {
                                       (void)sirkit::detail::checked_subtree_interval_size(std::numeric_limits<std::size_t>::max());
                                   });
    ExpectThrow<std::out_of_range>([]
                                   {
                                       SubtreeIntervals value(std::vector<int>{-2});
                                   });
    ExpectThrow<std::out_of_range>([]
                                   {
                                       SubtreeIntervals value(std::vector<int>{-1, 2});
                                   });
    ExpectThrow<std::invalid_argument>([]
                                       {
                                           SubtreeIntervals value(std::vector<int>{0});
                                       });
    ExpectThrow<std::invalid_argument>([]
                                       {
                                           SubtreeIntervals value(std::vector<int>{-1, 2, 1});
                                       });
#ifndef SKIP_RUNTIME_CHECKS
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)actual.vertex_at(-1);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)actual.range(n);
                                   });
#endif
}
}

int main()
{
    RandomForests();
    Boundaries();
}
