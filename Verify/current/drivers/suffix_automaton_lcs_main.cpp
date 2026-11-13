// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>
int main(){std::string first,second;verify_in>>first>>second;SuffixAutomaton<>automaton(first);auto match=automaton.longest_common_substring(second);verify_out<<match.first_start<<' '<<match.first_start+match.length<<' '<<match.second_start<<' '<<match.second_start+match.length<<'\n';}
