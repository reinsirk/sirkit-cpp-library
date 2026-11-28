// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Radix_Heap.cpp"

#include <cassert>
#include <limits>
#include <map>
#include <memory>
#include <random>

int main()
{
    using Key = std::uint64_t;
    constexpr auto kMax = std::numeric_limits<Key>::max();
    sirkit::RadixHeap<Key, int> heap(1);
    assert(heap.empty() && heap.size() == 0 && heap.lower_bound() == 1);
    heap.emplace(100, 0);
    assert(heap.top().first == 100 && heap.lower_bound() == 1);
    heap.emplace(2, 1);
    assert(heap.pop().first == 2 && heap.lower_bound() == 2);
    heap.emplace(kMax, 2);
    heap.emplace(100, 3);
    assert(heap.pop().first == 100 && heap.top().first == 100 && heap.pop().first == 100);
    assert(heap.pop().first == kMax && heap.empty());
    heap.emplace(kMax, 4);
    assert(heap.top().second == 4 && heap.pop().second == 4);
    heap.clear();
    assert(heap.lower_bound() == 0 && heap.empty());

    std::mt19937_64 rng(61286);
    constexpr auto kLarge = Key{1} << 63U;
    for (int test = 0; test < 4; ++test)
    {
        const Key base = test % 2 == 0 ? 0 : kLarge;
        heap.clear(base);
        std::multimap<Key, int> reference;
        for (int i = 0; i < 4500; ++i)
        {
            if (reference.empty() || rng() % 5 < 3)
            {
                const Key key = heap.lower_bound() | (rng() % (Key{1} << (1 + rng() % 62)));
                heap.emplace(key, i);
                reference.emplace(key, i);
            }
            else
            {
                const Key previous = heap.lower_bound();
                assert(heap.top().first == reference.begin()->first && heap.lower_bound() == previous);
                const auto [key, value] = heap.pop();
                assert(key == reference.begin()->first && heap.lower_bound() == key);
                const auto ending = reference.upper_bound(key);
                auto match = reference.begin();
                while (match != ending && match->second != value)
                {
                    ++match;
                }
                assert(match != ending);
                reference.erase(match);
            }
            assert(heap.size() == reference.size());
        }
        auto copied = heap;
        auto moved = std::move(copied);
        assert(copied.empty() && copied.lower_bound() == 0 && moved.size() == heap.size());
        copied = std::move(moved);
        for (const auto &item : reference)
        {
            assert(copied.pop().first == item.first);
        }
        assert(copied.empty());
    }

    sirkit::RadixHeap<unsigned char, std::unique_ptr<int>> movable;
    movable.emplace(127, std::make_unique<int>(33));
    movable.emplace(10, std::make_unique<int>(44));
    assert(*movable.pop().second == 44 && *movable.pop().second == 33);

#ifndef SKIP_RUNTIME_CHECKS
    heap.clear(50);
    bool rejected = false, empty_top = false, empty_pop = false;
    try
    {
        heap.emplace(49, -1);
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    try
    {
        (void)heap.top();
    }
    catch (const std::out_of_range &)
    {
        empty_top = true;
    }
    try
    {
        (void)heap.pop();
    }
    catch (const std::out_of_range &)
    {
        empty_pop = true;
    }
    assert(rejected && empty_top && empty_pop && heap.lower_bound() == 50);
#endif
}
