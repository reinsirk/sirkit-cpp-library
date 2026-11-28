// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Run_Length_Queue.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

using sirkit::RunLengthQueue;
using Count = std::uint64_t;

template <typename Error, typename Operation>
void expect_error(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Error &)
    {
        caught = true;
    }
    assert(caught);
}

struct Fragile
{
    inline static bool fail_copy = false;
    int value;

    explicit Fragile(int initial) : value(initial)
    {
    }

    Fragile(const Fragile &other) : value(other.value)
    {
        if (fail_copy)
        {
            throw std::logic_error("copy failed");
        }
    }

    Fragile(Fragile &&) = default;
    friend bool operator==(const Fragile &, const Fragile &) = default;
};

void boundary_cases()
{
    constexpr Count maximum = std::numeric_limits<Count>::max();
    RunLengthQueue<int> queue;
    assert(queue.empty() && queue.size() == 0 && queue.front_count() == 0 && queue.run_count() == 0);
    queue.push_back(9, 0);
    queue.pop_front(0);
    queue.consume_front(0, [](int, Count) { assert(false); });
    assert(queue.empty());

    queue.push_back(1, maximum);
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::length_error>([&] { queue.push_back(2); });
#endif
    assert(queue.size() == maximum && queue.front() == 1);
    queue.pop_front(maximum - 2);
    queue.push_back(2, 3);
    assert(queue.size() == 5 && queue.front_count() == 2 && queue.run_count() == 2);
    queue.pop_front(3);
    assert(queue.size() == 2 && queue.front() == 2 && queue.front_count() == 2 && queue.run_count() == 1);
    queue.push_back(2, maximum - 2);
    assert(queue.size() == maximum && queue.front_count() == maximum && queue.run_count() == 1);

    RunLengthQueue<int> copy(queue);
    queue.pop_front(maximum);
    assert(queue.empty() && !copy.empty());
    const auto &copy_self = copy;
    copy = copy_self;
    queue = copy;
    assert(queue.size() == maximum && queue.front_count() == maximum && queue.front() == 2);
    RunLengthQueue<int> moved(std::move(queue));
    assert(queue.empty() && queue.front_count() == 0 && moved.size() == maximum);
    const auto moved_self = &moved;
    moved = std::move(*moved_self);
    assert(moved.size() == maximum);
    queue = std::move(moved);
    assert(moved.empty() && !queue.empty());
    queue.pop_front(maximum);
    queue.push_back(8);
    queue.pop_front();
    assert(queue.empty());

    RunLengthQueue<std::unique_ptr<int>> unique;
    unique.push_back(std::make_unique<int>(17), 5);
    auto next(std::move(unique));
    assert(unique.empty() && *next.front() == 17 && next.front_count() == 5);
}

void exception_cases()
{
    RunLengthQueue<int> queue;
    queue.push_back(3, 3);
    queue.push_back(8, 2);
    std::vector<std::pair<int, Count>> visited;
    expect_error<std::logic_error>([&]
    {
        queue.consume_front(5, [&](int value, Count count)
        {
            if (value == 8)
            {
                throw std::logic_error("visitor failed");
            }
            visited.emplace_back(value, count);
        });
    });
    const std::vector<std::pair<int, Count>> only_first{{3, 3}};
    assert(visited == only_first && queue.front() == 8 && queue.size() == 2 && queue.front_count() == 2);
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::out_of_range>([&]
    {
        queue.consume_front(3, [&](int value, Count count)
        {
            visited.emplace_back(value, count);
        });
    });
    assert(visited == only_first && queue.size() == 2);
    queue.pop_front(2);
    expect_error<std::out_of_range>([&] { static_cast<void>(queue.front()); });
#endif

    RunLengthQueue<Fragile> from, to;
    from.push_back(Fragile(1), 4);
    to.push_back(Fragile(2), 7);
    to.pop_front(3);
    Fragile::fail_copy = true;
    expect_error<std::logic_error>([&] { to = from; });
    assert(to.size() == 4 && to.front().value == 2 && from.size() == 4 && from.front().value == 1);
    Fragile::fail_copy = false;
    to = from;
    assert(to.size() == 4 && to.front().value == 1);
}

void randomized_cases()
{
    std::mt19937 random(0x819236);
    for (int repeat = 0; repeat < 90; ++repeat)
    {
        RunLengthQueue<int> queue;
        std::deque<int> single;
        for (int operation = 0; operation < 250; ++operation)
        {
            if (single.empty() || random() % 2 == 0)
            {
                const Count count = random() % 15;
                const int value = static_cast<int>(random() % 7) - 3;
                queue.push_back(value, count);
                single.insert(single.end(), static_cast<std::size_t>(count), value);
            }
            else
            {
                const Count count = random() % (single.size() + 1);
                std::vector<int> got;
                queue.consume_front(count, [&](int value, Count copies)
                {
                    assert(got.empty() || got.back() != value);
                    got.insert(got.end(), static_cast<std::size_t>(copies), value);
                });
                assert(got.size() == count);
                for (const auto value : got)
                {
                    assert(single.front() == value);
                    single.pop_front();
                }
            }
            assert(queue.size() == single.size() && queue.empty() == single.empty());
            if (!single.empty())
            {
                assert(queue.front() == single.front());
                std::size_t runs = 1, first_count = 1;
                for (std::size_t i = 1; i < single.size(); ++i)
                {
                    if (single[i] != single[i - 1])
                    {
                        ++runs;
                    }
                    if (runs == 1)
                    {
                        ++first_count;
                    }
                }
                assert(queue.run_count() == runs && queue.front_count() == first_count);
            }
        }
    }
}

int main()
{
    boundary_cases();
    exception_cases();
    randomized_cases();
}
