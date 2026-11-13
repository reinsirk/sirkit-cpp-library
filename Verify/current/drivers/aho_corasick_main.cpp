// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>
int main() {
    int n{}; verify_in >> n;
    AhoCorasick<> automaton;
    for (int i = 0; i < n; ++i) {
        std::string s; verify_in >> s;
        automaton.add(s);
    }
    automaton.build();
    verify_out << automaton.state_count() << '\n';
    for (int state = 1; state < automaton.state_count(); ++state)
        verify_out << automaton.parent(state) << ' ' << automaton.failure(state) << '\n';
    for (int i = 0; i < n; ++i) verify_out << (i ? " " : "") << automaton.pattern_state(i);
    verify_out << '\n';
}
