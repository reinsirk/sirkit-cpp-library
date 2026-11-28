// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Indexed_Priority_Queue.cpp"

#include <array>
#include <cassert>
#include <limits>
#include <optional>
#include <random>

template <bool Largest>
void Check()
{
    constexpr std::size_t kLimit = 53;
    sirkit::IndexedPriorityQueue<int, Largest> queue(kLimit);
    std::array<std::optional<int>, kLimit> reference{};
    std::mt19937 rng(127525);
    const auto best = [&]()
    {
        std::optional<std::size_t> best_id;
        for (std::size_t id = 0; id < kLimit; ++id)
        {
            if (!reference[id])
            {
                continue;
            }
            if (!best_id || (Largest ? *reference[id] > *reference[*best_id] : *reference[id] < *reference[*best_id]))
            {
                best_id = id;
            }
        }
        return best_id;
    };
    for (int step = 0; step < 7500; ++step)
    {
        const std::size_t id = rng() % kLimit;
        const int priority = static_cast<int>(rng() % 31) - 15;
        const auto operation = rng() % 23;
        if (operation < 17)
        {
            queue.set(id, priority);
            reference[id] = priority;
        }
        else if (operation < 21)
        {
            assert(queue.erase(id) == reference[id].has_value());
            reference[id].reset();
        }
        else if (const auto front = best())
        {
            assert((queue.pop() == std::pair{*front, *reference[*front]}));
            reference[*front].reset();
        }
        if (step % 257 == 0)
        {
            queue.clear();
            reference.fill(std::nullopt);
        }
        const auto front = best();
        assert(queue.empty() == !front.has_value());
        if (front)
        {
            assert((queue.top() == std::pair{*front, *reference[*front]}));
        }
        std::size_t size = 0;
        for (std::size_t j = 0; j < kLimit; ++j)
        {
            assert(queue.contains(j) == reference[j].has_value());
            if (reference[j])
            {
                assert(queue.priority(j) == *reference[j]);
                ++size;
            }
        }
        assert(queue.size() == size && queue.id_limit() == kLimit);
    }

    queue.clear();
    queue.set(5, 0);
    queue.set(2, 0);
    auto copied = queue;
    assert(copied.pop().first == 2 && copied.pop().first == 5);
    assert(queue.size() == 2);
    queue = std::move(copied);
    assert(queue.empty() && queue.id_limit() == kLimit);
    queue.set(1, std::numeric_limits<int>::min());
    queue.set(2, std::numeric_limits<int>::max());
    assert(queue.pop().first == (Largest ? 2U : 1U));
    queue.clear();
#ifndef SKIP_RUNTIME_CHECKS
    bool bad_id = false, missing = false, bad_top = false;
    try
    {
        queue.set(kLimit, 0);
    }
    catch (const std::out_of_range &)
    {
        bad_id = true;
    }
    try
    {
        (void)queue.priority(0);
    }
    catch (const std::out_of_range &)
    {
        missing = true;
    }
    try
    {
        (void)queue.pop();
    }
    catch (const std::out_of_range &)
    {
        bad_top = true;
    }
    assert(bad_id && missing && bad_top && queue.empty());
#endif
}

int main()
{
    sirkit::IndexedPriorityQueue<int> empty(0);
    assert(empty.empty() && empty.id_limit() == 0);
    Check<false>();
    Check<true>();
}
