// SPDX-License-Identifier: CC0-1.0
#include "../String/Aho_Corasick.cpp"
#include <iostream>

using namespace sirkit;
int main() {
    AhoCorasick<> automaton;
    automaton.add("he"); automaton.add("she"); automaton.add("he"); automaton.add("");
    automaton.build();
    std::cout << automaton.count_total("shehe") << '\n';
    for (auto count : automaton.match_counts("shehe")) std::cout << count << ' ';
    std::cout << '\n';
    automaton.for_each_match("she", [](int id, std::size_t begin, std::size_t end) {
        std::cout << id << ':' << begin << ',' << end << ' ';
    });
    std::cout << '\n';
    const auto matches = automaton.find_all("he");
    std::cout << matches.size() << ' ' << automaton.parent(automaton.pattern_state(0)) << '\n';
    automaton.clear(); automaton.add("a"); automaton.build();
    std::cout << automaton.count_total("aaa") << '\n';
}
