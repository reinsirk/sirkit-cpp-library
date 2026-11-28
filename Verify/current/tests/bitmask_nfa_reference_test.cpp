// SPDX-License-Identifier: CC0-1.0
#include "String/Bitmask_NFA.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <queue>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
struct Arc
{
    std::size_t from, to;
    int symbol;
};

bool accepts(std::span<const Arc> arcs, std::span<const std::size_t> word,
             std::span<const std::uint8_t> accepting, std::size_t initial)
{
    const auto n = accepting.size();
    std::vector<std::uint8_t> visited(n * (word.size() + 1));
    std::queue<std::pair<std::size_t, std::size_t>> pending;
    pending.emplace(initial, 0);
    visited[initial] = 1;
    while (!pending.empty())
    {
        const auto [source, position] = pending.front();
        pending.pop();
        if (position == word.size() && accepting[source] != 0)
        {
            return true;
        }
        for (const auto [from, to, symbol] : arcs)
        {
            if (from != source ||
                (symbol != -1 && (position == word.size() || word[position] != static_cast<std::size_t>(symbol))))
            {
                continue;
            }
            const auto next = position + (symbol != -1);
            if (!visited[next * n + to])
            {
                visited[next * n + to] = 1;
                pending.emplace(to, next);
            }
        }
    }
    return false;
}

template <class Exception, class Action>
void expect_failure(Action action)
{
    try
    {
        action();
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
}

int main()
{
    std::mt19937 rng(484);
    for (int test = 0; test < 150; ++test)
    {
        const std::size_t n = 1 + rng() % 8;
        const std::size_t initial = rng() % n;
        sirkit::BitmaskNfa<3> nfa(n, initial);
        std::vector<std::uint8_t> accepting(n);
        std::vector<Arc> graph;
        for (std::size_t state = 0; state < n; ++state)
        {
            accepting[state] = rng() % 2;
            nfa.set_accepting(state, accepting[state] != 0);
        }
        for (std::size_t i = 0, m = rng() % (4 * n); i < m; ++i)
        {
            Arc arc{rng() % n, rng() % n, static_cast<int>(rng() % 4) - 1};
            graph.push_back(arc);
            if (arc.symbol == -1)
            {
                nfa.add_epsilon(arc.from, arc.to);
            }
            else
            {
                nfa.add_transition(arc.from, static_cast<std::size_t>(arc.symbol), arc.to);
            }
        }
        const auto dfa = nfa.determinize();
        assert(1 <= dfa.size() && dfa.size() <= (std::size_t{1} << n));
        for (std::size_t length = 0; length <= 5; ++length)
        {
            std::vector<std::uint64_t> choices(length);
            for (auto &mask : choices)
            {
                mask = rng() % 8;
            }
            std::vector<std::size_t> word(length);
            int count = 0;
            const auto enumerate = [&](const auto &self, std::size_t i, bool allowed) -> void
            {
                if (i == length)
                {
                    const bool expected = accepts(graph, word, accepting, initial);
                    assert(dfa.accepts(word) == expected);
                    std::size_t state = 0;
                    for (const auto symbol : word)
                    {
                        state = dfa.transition(state, symbol);
                    }
                    assert(dfa.accepting(state) == expected);
                    count += expected && allowed;
                    return;
                }
                for (std::size_t symbol = 0; symbol < 3; ++symbol)
                {
                    word[i] = symbol;
                    self(self, i + 1, allowed && ((choices[i] >> symbol) & 1) != 0);
                }
            };
            enumerate(enumerate, 0, true);
            assert(dfa.count_matches<int>(choices) == count);
        }
    }

    sirkit::BitmaskNfa<64> high(64, 63);
    high.add_epsilon(63, 0);
    high.add_epsilon(0, 63);
    high.add_transition(0, 63, 63);
    high.set_accepting(63);
    auto top = high.determinize();
    assert(top.accepts(std::array<std::size_t, 1>{63}));
    assert(top.count_matches<int>(std::array<std::uint64_t, 1>{std::uint64_t{1} << 63}) == 1);
    assert(top.count_matches<int>(std::array<std::uint64_t, 2>{top.kAllSymbols, top.kAllSymbols}) == 1);
    auto moved_top = std::move(top);
    assert(moved_top.accepts({}));
    // The moved-from object may be empty. If so it represents an empty language.
    if (top.size() == 0)
    {
        assert(!top.accepts({}));
        assert(top.count_matches<int>({}) == 0);
    }
    auto moved_high = std::move(high);
    assert(moved_high.determinize().accepts({}));
    // It is valid to determinize after a move, whatever state the vector keeps.
    const auto moved_from_dfa = high.determinize();
    if (moved_from_dfa.size() == 0)
    {
        assert(!moved_from_dfa.accepts({}));
    }

    sirkit::BitmaskNfa<2> guess(9);
    guess.add_transition(0, 0, 0);
    guess.add_transition(0, 1, 0);
    guess.add_transition(0, 1, 1);
    for (std::size_t source = 1; source < 8; ++source)
    {
        guess.add_transition(source, 0, source + 1);
        guess.add_transition(source, 1, source + 1);
    }
    guess.set_accepting(8);
    expect_failure<std::length_error>([&]
                                      {
                                          (void)guess.determinize(255);
                                      });
    const auto all = guess.determinize(256);
    assert(all.size() == 256);
    assert(all.count_matches<int>(std::vector<std::uint64_t>(16, 3)) == (1 << 15));
    expect_failure<std::invalid_argument>([]
                                          {
                                              const sirkit::BitmaskNfa<2> invalid(0);
                                          });
    expect_failure<std::invalid_argument>([]
                                          {
                                              const sirkit::BitmaskNfa<2> invalid(65);
                                          });
    expect_failure<std::invalid_argument>([]
                                          {
                                              const sirkit::BitmaskNfa<2> invalid(3, 3);
                                          });
    expect_failure<std::out_of_range>([&]
                                      {
                                          guess.add_epsilon(0, 9);
                                      });
    expect_failure<std::out_of_range>([&]
                                      {
                                          guess.add_transition(0, 2, 0);
                                      });
    expect_failure<std::out_of_range>([&]
                                      {
                                          (void)all.transition(0, 2);
                                      });
    expect_failure<std::out_of_range>([&]
                                      {
                                          (void)all.transition(all.size(), 0);
                                      });
    expect_failure<std::out_of_range>([&]
                                      {
                                          (void)all.accepting(all.size());
                                      });
    expect_failure<std::out_of_range>([&]
                                      {
                                          (void)all.accepts(std::array<std::size_t, 1>{2});
                                      });
    expect_failure<std::out_of_range>([&]
                                      {
                                          (void)all.count_matches<int>(std::array<std::uint64_t, 1>{4});
                                      });
}
