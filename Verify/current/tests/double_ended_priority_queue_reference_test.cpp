// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Double_Ended_Priority_Queue.cpp"
#include <cassert>
#include <functional>
#include <random>
#include <set>
#include <vector>

int main()
{
    std::mt19937 random(19094);
    for (int trial = 0; trial < 100; ++trial)
    {
        std::vector<int> values(trial);
        for (auto &item : values)
        {
            item = static_cast<int>(random() % 80) - 40;
        }
        sirkit::DoubleEndedPriorityQueue<int> queue(values);
        std::multiset<int> expected(values.begin(), values.end());
        for (int request = 0; request < 1000; ++request)
        {
            const auto kind = random() % 5;
            if (expected.empty() || kind < 3)
            {
                const auto value = static_cast<int>(random() % 83) - 42;
                queue.push(value);
                expected.insert(value);
            }
            else if (kind == 3)
            {
                assert(queue.pop_min() == *expected.begin());
                expected.erase(expected.begin());
            }
            else
            {
                const auto last = std::prev(expected.end());
                assert(queue.pop_max() == *last);
                expected.erase(last);
            }
            assert(queue.size() == expected.size() && queue.empty() == expected.empty());
            if (!expected.empty())
            {
                assert(queue.min() == *expected.begin() && queue.max() == *expected.rbegin());
            }
        }
        for (auto it = expected.begin(); it != expected.end(); ++it)
        {
            assert(queue.pop_min() == *it);
        }
        assert(queue.empty());
    }
    sirkit::DoubleEndedPriorityQueue<int, std::greater<int>> reverse({1, 2, 1, 3});
    reverse.push(0);
    assert(reverse.pop_min() == 3 && reverse.pop_max() == 0);
#ifndef SKIP_RUNTIME_CHECKS
    const auto invalid = [](auto operation)
    {
        try
        {
            operation();
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    };
    sirkit::DoubleEndedPriorityQueue<int> empty;
    invalid([&]()
            {
                return empty.min();
            });
    invalid([&]()
            {
                return empty.max();
            });
    invalid([&]()
            {
                return empty.pop_min();
            });
    invalid([&]()
            {
                return empty.pop_max();
            });
#endif
}
