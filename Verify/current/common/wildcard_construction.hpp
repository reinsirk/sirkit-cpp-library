// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_VERIFY_WILDCARD_CONSTRUCTION_DEFINED
#define SIRKIT_VERIFY_WILDCARD_CONSTRUCTION_DEFINED

#include <cstddef>
#include <string_view>
#include <vector>

namespace verify_wildcard
{
inline sirkit::BitmaskDfa<2> automaton(std::string_view x, std::string_view y)
{
    sirkit::BitmaskNfa<2> nfa(x.size() + y.size());
    nfa.set_accepting(0);
    nfa.set_accepting(1);
    std::size_t available_state = 2;
    const auto add = [&](std::size_t source, char allowed, std::size_t destination)
    {
        for (std::size_t symbol = 0; symbol < 2; ++symbol)
        {
            if (allowed == '?' || allowed == static_cast<char>('0' + symbol))
            {
                nfa.add_transition(source, symbol, destination);
            }
        }
    };
    const auto path = [&](std::string_view word, std::size_t end, bool allowed_after_y)
    {
        std::size_t source = 0;
        for (std::size_t i = 0; i < word.size(); ++i)
        {
            const auto next = i + 1 == word.size() ? end : available_state++;
            add(source, word[i], next);
            if (i == 0 && allowed_after_y)
            {
                add(1, word[i], next);
            }
            source = next;
        }
    };
    path(x, 0, true);
    path(y, 1, false);
    return nfa.determinize();
}

inline std::vector<sirkit::BitmaskDfa<2>::Mask> masks(std::string_view text)
{
    std::vector<sirkit::BitmaskDfa<2>::Mask> choices;
    choices.reserve(text.size());
    for (const char c : text)
    {
        choices.push_back(c == '?' ? 3 : 1ULL << (c - '0'));
    }
    return choices;
}
}

#endif
