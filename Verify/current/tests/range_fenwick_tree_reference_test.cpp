// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Range_Fenwick_Tree.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

void CheckWraparound()
{
    sirkit::RangeFenwickTree<std::int64_t> wide(4);
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    wide.set(0, kMax);
    wide.set(1, kMin);
    wide.add(1, 3, kMin);
    assert(wide.get(0) == kMax && wide.get(1) == 0 && wide.sum(0, 4) == -1);
    wide.add(0, 2, 1);
    assert(wide.get(0) == kMin && wide.sum(0, 4) == 1);

    std::vector<std::uint16_t> high(100000, 65535);
    sirkit::RangeFenwickTree<std::uint16_t> small(high);
    small.add(3, 100000, 65535);
    assert(small.get(0) == 65535 && small.get(99999) == 65534);
    assert(small.sum(3, 100000) == static_cast<std::uint16_t>(99997U * 65534U));
}

int main()
{
    CheckWraparound();
    sirkit::RangeFenwickTree<> empty;
    empty.add(0, 0, 10);
    assert(empty.empty() && empty.size() == 0 && empty.sum(0, 0) == 0);
    std::mt19937_64 random(4061);
    for (int n : {1, 2, 9, 32, 66, 103})
    {
        std::vector<std::int64_t> values(n);
        for (auto &value : values)
        {
            value = static_cast<int>(random() % 61) - 30;
        }
        sirkit::RangeFenwickTree<> tree(values);
        for (int step = 0; step < 2000; ++step)
        {
            const auto change = static_cast<int>(random() % 501) - 250;
            int first = random() % (n + 1);
            int last = random() % (n + 1);
            if (first > last)
            {
                std::swap(first, last);
            }
            int index = random() % n;
            if (step % 3 == 0)
            {
                tree.set(index, change);
                values[index] = change;
            }
            else
            {
                tree.add(first, last, change);
                for (int i = first; i < last; ++i)
                {
                    values[i] += change;
                }
            }
            assert(tree.size() == n && !tree.empty());
            assert(tree.get(index) == values[index]);
            assert(tree.sum(first, last) == std::accumulate(values.begin() + first, values.begin() + last, std::int64_t{0}));
            if (step % 31 == 0)
            {
                auto independent = tree;
                independent.set(index, independent.get(index) + 1);
                auto moved = std::move(independent);
                assert(independent.empty() && tree.get(index) == values[index] && moved.get(index) == values[index] + 1);
            }
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    auto out_of_range = [](auto action)
    {
        try
        {
            action();
        }
        catch (const std::out_of_range &)
        {
            return true;
        }
        return false;
    };
    sirkit::RangeFenwickTree<> tree(3);
    assert(out_of_range([&]
                        {
                            tree.set(-1, 0);
                        }));
    assert(out_of_range([&]
                        {
                            (void)tree.get(3);
                        }));
    assert(out_of_range([&]
                        {
                            tree.add(-1, 3, 1);
                        }));
    assert(out_of_range([&]
                        {
                            (void)tree.sum(2, 1);
                        }));
    assert(out_of_range([&]
                        {
                            tree.add(0, 4, 1);
                        }));
    assert(tree.sum(0, 3) == 0);
#endif
}
