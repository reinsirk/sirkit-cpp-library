// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_PARALLEL_BINARY_SEARCH_DEFINED
#include "../../../Algorithm/Parallel_Binary_Search.cpp"
#endif
#include <cassert>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
void check(const std::vector<int> &events, const std::vector<int> &targets)
{
    std::size_t applied = 0;
    int total = -1;
    const auto actual = sirkit::parallel_binary_search(
        events.size(), targets.size(),
        [&] {
            applied = 0;
            total = 0;
        },
        [&](std::size_t step) {
            assert(step == applied && step < events.size());
            ++applied;
            total += events[step];
        },
        [&](std::size_t query) {
            assert(total >= 0 && query < targets.size());
            return total >= targets[query];
        });
    assert(actual.size() == targets.size());
    for (std::size_t i = 0; i < targets.size(); ++i)
    {
        std::size_t steps = 0;
        int value = 0;
        while (steps < events.size() && value < targets[i])
        {
            value += events[steps++];
        }
        assert(actual[i] == (value < targets[i] ? events.size() + 1 : steps));
    }
    if (targets.empty())
    {
        assert(total == -1);
    }
}
} // namespace

int main()
{
    check({}, {});
    check({}, {-1, 0, 1});
    check({2, 0, 1, 3}, {3, 0, 0, 7, 3, 2, 6, -1});
    std::mt19937 random(679223);
    for (int trial = 0; trial < 1700; ++trial)
    {
        std::vector<int> events(random() % 75);
        int total = 0;
        for (auto &event : events)
        {
            event = static_cast<int>(random() % 11);
            total += event;
        }
        std::vector<int> targets(random() % 65);
        for (auto &target : targets)
        {
            target = static_cast<int>(random() % (total + 5)) - 2;
        }
        check(events, targets);
    }

    constexpr std::size_t kScale = 200'000;
    std::size_t completed = 0;
    const auto answer = sirkit::parallel_binary_search(
        kScale, kScale + 2, [&] { completed = 0; },
        [&](std::size_t step) {
            assert(step == completed);
            ++completed;
        },
        [&](std::size_t target) { return completed >= target; });
    for (std::size_t i = 0; i < answer.size(); ++i)
    {
        assert(answer[i] == i);
    }

    bool thrown = false;
    try
    {
        (void)sirkit::parallel_binary_search(std::numeric_limits<std::size_t>::max(), 1,
                                            [] {}, [](std::size_t) {}, [](std::size_t) { return false; });
    }
    catch (const std::length_error &)
    {
        thrown = true;
    }
    assert(thrown);
}
