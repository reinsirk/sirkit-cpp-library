// SPDX-License-Identifier: CC0-1.0
#include "String/DFA_Minimization.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
void compare_to_refinement(std::size_t alphabet, std::span<const std::size_t> transitions,
                           std::span<const std::uint8_t> accepts, std::size_t start)
{
    const auto n = accepts.size();
    auto minimized = sirkit::minimize_dfa(alphabet, transitions, accepts, start);
    assert(minimized.alphabet() == alphabet && minimized.state_map()[start] == 0);
    const auto mapped = minimized.state_map();
    std::vector<std::uint8_t> reachable(n), equal(n * n);
    reachable[start] = 1;
    for (std::size_t iteration = 0; iteration < n; ++iteration)
    {
        for (std::size_t state = 0; state < n; ++state)
        {
            if (reachable[state])
            {
                for (std::size_t c = 0; c < alphabet; ++c)
                {
                    reachable[transitions[state * alphabet + c]] = 1;
                }
            }
            for (std::size_t other = 0; other < n; ++other)
            {
                equal[state * n + other] = (accepts[state] != 0) == (accepts[other] != 0);
            }
        }
    }
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (std::size_t a = 0; a < n; ++a)
        {
            for (std::size_t b = 0; b < n; ++b)
            {
                if (!equal[a * n + b])
                {
                    continue;
                }
                for (std::size_t c = 0; c < alphabet; ++c)
                {
                    const auto next_a = transitions[a * alphabet + c];
                    const auto next_b = transitions[b * alphabet + c];
                    if (!equal[next_a * n + next_b])
                    {
                        equal[a * n + b] = 0;
                        changed = true;
                        break;
                    }
                }
            }
        }
    }
    std::vector<std::uint8_t> used(minimized.size());
    for (std::size_t a = 0; a < n; ++a)
    {
        if (!reachable[a])
        {
            assert(mapped[a] == sirkit::DfaMinimization::kUnreachable);
            continue;
        }
        assert(mapped[a] < minimized.size());
        used[mapped[a]] = 1;
        assert(minimized.accepting(mapped[a]) == (accepts[a] != 0));
        for (std::size_t c = 0; c < alphabet; ++c)
        {
            assert(minimized.transition(mapped[a], c) == mapped[transitions[a * alphabet + c]]);
        }
        for (std::size_t b = 0; b < n; ++b)
        {
            if (reachable[b])
            {
                assert((mapped[a] == mapped[b]) == (equal[a * n + b] != 0));
            }
        }
    }
    assert(std::all_of(used.begin(), used.end(), [](auto flag)
                       {
                           return flag != 0;
                       }));
}

template <class Error, class Action>
void throws(Action action)
{
    try
    {
        action();
        assert(false);
    }
    catch (const Error &)
    {
    }
}
}

int main()
{
    using sirkit::minimize_dfa;
    compare_to_refinement(0, {}, std::array<std::uint8_t, 2>{0, 1}, 1);
    compare_to_refinement(1, std::array<std::size_t, 2>{1, 0}, std::array<std::uint8_t, 2>{0, 0}, 0);
    compare_to_refinement(1, std::array<std::size_t, 2>{1, 0}, std::array<std::uint8_t, 2>{3, 2}, 0);
    std::mt19937 random(51233);
    for (int test = 0; test < 2000; ++test)
    {
        const std::size_t n = 1 + random() % 22;
        const std::size_t alphabet = random() % 7;
        std::vector<std::size_t> transitions(n * alphabet);
        std::vector<std::uint8_t> accepts(n);
        for (auto &v : transitions)
        {
            v = random() % n;
        }
        for (auto &flag : accepts)
        {
            flag = static_cast<std::uint8_t>(random() % 3 == 0);
        }
        compare_to_refinement(alphabet, transitions, accepts, random() % n);
    }

    constexpr std::size_t n = 200000;
    std::vector<std::size_t> transitions(n * 2);
    std::vector<std::uint8_t> accepts(n);
    for (std::size_t v = 0; v < n; ++v)
    {
        transitions[2 * v] = (v + 2) % n;
        transitions[2 * v + 1] = v ^ 1;
        accepts[v] = v < 2;
    }
    const auto cycle = minimize_dfa(2, transitions, accepts, 0);
    assert(cycle.size() == n / 2);
    assert(cycle.accepting(0));
    for (std::size_t v = 0; v < n; v += 2)
    {
        const auto map = cycle.state_map();
        assert(map[v] == map[v + 1]);
        assert(cycle.transition(map[v], 1) == map[v]);
        assert(cycle.transition(map[v], 0) == map[(v + 2) % n]);
    }

    const std::array<std::uint8_t, 2> finals{0, 1};
    throws<std::invalid_argument>([]
                                  {
                                      (void)minimize_dfa(0, {}, {});
                                  });
    throws<std::invalid_argument>([&]
                                  {
                                      (void)minimize_dfa(0, {}, finals, 2);
                                  });
    throws<std::invalid_argument>([&]
                                  {
                                      (void)minimize_dfa(1, {}, finals);
                                  });
    throws<std::invalid_argument>([&]
                                  {
                                      (void)minimize_dfa(0, std::array<std::size_t, 1>{0}, finals);
                                  });
    throws<std::out_of_range>([&]
                              {
                                  (void)minimize_dfa(1, std::array<std::size_t, 2>{1, 2}, finals);
                              });
    throws<std::out_of_range>([&]
                              {
                                  (void)cycle.accepting(cycle.size());
                              });
    throws<std::out_of_range>([&]
                              {
                                  (void)cycle.transition(cycle.size(), 0);
                              });
    throws<std::out_of_range>([&]
                              {
                                  (void)cycle.transition(0, cycle.alphabet());
                              });
}
