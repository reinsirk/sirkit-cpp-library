// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>
int main(){std::string text;verify_in>>text;SuffixAutomaton<>automaton(text);verify_out<<automaton.distinct_substring_count()<<'\n';}
