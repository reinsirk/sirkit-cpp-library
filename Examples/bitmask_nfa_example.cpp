// SPDX-License-Identifier: CC0-1.0
#include "../String/Bitmask_NFA.cpp"

#include <array>
#include <iostream>

int main()
{
    sirkit::BitmaskNfa<2> nfa(2);
    nfa.add_transition(0, 0, 0);
    nfa.add_transition(0, 1, 0);
    nfa.add_transition(0, 1, 1);
    nfa.set_accepting(1);

    const auto dfa = nfa.determinize();
    const std::array<sirkit::BitmaskDfa<2>::Mask, 3> choices{3, 3, 3};
    std::cout << dfa.count_matches<int>(choices) << '\n'; // 4
}
