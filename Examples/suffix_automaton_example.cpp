// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <iostream>
#include "../String/Suffix_Automaton.cpp"
int main(){
 SuffixAutomaton<>automaton("ababa");
 std::cout<<automaton.distinct_substring_count()<<' '<<automaton.occurrences("aba")<<'\n'; // 9 2
 std::cout<<automaton.contains("bab")<<' '<<automaton.occurrences("")<<'\n'; // 1 6
 automaton.reserve(100);automaton.append(static_cast<std::uint8_t>('c'));
 auto match=automaton.longest_common_substring("zzbabab");std::cout<<match.first_start<<' '<<match.second_start<<' '<<match.length<<'\n'; // 1 2 4
 auto copy=automaton;copy.append("a");std::cout<<automaton.size()<<' '<<copy.size()<<'\n'; // 6 7
 automaton.reset("aaa");std::cout<<automaton.distinct_substring_count()<<' '<<automaton.occurrences("a")<<'\n'; // 3 3
 automaton.clear();std::cout<<automaton.empty()<<' '<<automaton.occurrences("")<<'\n'; // 1 1
}
