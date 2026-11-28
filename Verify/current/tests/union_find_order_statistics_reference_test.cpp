// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Union_Find_Order_Statistics.cpp"

#include <cassert>
#include <random>
#include <stdexcept>
#include <string>

int main(int argc, char **)
{
    if (argc > 1)
    {
        sirkit::UnionFindOrderStatistics<int> empty({});
        static_cast<void>(empty.root(0));
        return 0;
    }
    sirkit::UnionFindOrderStatistics<int> empty({});
    assert(empty.size() == 0 && empty.component_count() == 0);
#ifndef SKIP_RUNTIME_CHECKS
    bool stopped = false;
    try
    {
        static_cast<void>(empty.kth(0, 0));
    }
    catch (const std::out_of_range &)
    {
        stopped = true;
    }
    assert(stopped);
#endif

    const std::vector<std::string> labels{"orange", "apple", "orange"};
    sirkit::UnionFindOrderStatistics named(labels);
    assert(named.merge(0, 1) && named.merge(1, 2));
    auto independent = named;
    independent = named = sirkit::UnionFindOrderStatistics(labels);
    assert(named.size(0) == 1 && independent.size(2) == 1);
    assert(independent.merge(1, 2));
    assert(independent.kth(2, 0) == "apple");
    assert(independent.count_less(2, std::string("orange")) == 1);
    assert(named.root(2) != named.root(1));

    std::mt19937 generator(337872);
    for (int trial = 0; trial < 350; ++trial)
    {
        const int n = 1 + static_cast<int>(generator() % 34);
        std::vector<int> keys(n), representatives(n);
        for (int i = 0; i < n; ++i)
        {
            keys[i] = static_cast<int>(generator() % 13) - 6;
            representatives[i] = i;
        }
        sirkit::UnionFindOrderStatistics tree(keys);
        int total_components = n;
        for (int query = 0; query < 95; ++query)
        {
            const int a = generator() % n, b = generator() % n;
            const int x = static_cast<int>(generator() % 17) - 8;
            if (query % 3 == 0)
            {
                const int first = representatives[a], second = representatives[b];
                assert(tree.merge(a, b) == (first != second));
                if (first != second)
                {
                    std::replace(representatives.begin(), representatives.end(), first, second);
                    --total_components;
                }
            }
            assert(tree.component_count() == total_components);
            std::vector<int> sorted;
            for (int i = 0; i < n; ++i)
            {
                if (representatives[i] == representatives[a])
                {
                    sorted.push_back(keys[i]);
                }
            }
            std::sort(sorted.begin(), sorted.end());
            assert(tree.issame(a, b) == (representatives[a] == representatives[b]));
            assert(tree.size(a) == static_cast<int>(sorted.size()));
            assert(!tree.kth(a, -1) && !tree.kth(a, sorted.size()));
            for (int k = 0; k < static_cast<int>(sorted.size()); ++k)
            {
                assert(tree.kth(a, k) == sorted[k]);
            }
            assert(tree.count(a, x) == std::count(sorted.begin(), sorted.end(), x));
            assert(tree.count_less(a, x) == std::lower_bound(sorted.begin(), sorted.end(), x) - sorted.begin());
        }
    }
}
