// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>
int main() {
    std::string text; verify_in >> text;
    int q{}; verify_in >> q;
    AhoCorasick<> automaton;
    for (int i = 0; i < q; ++i) {
        std::string pattern; verify_in >> pattern;
        automaton.add(pattern);
    }
    automaton.build();
    for (auto count : automaton.match_counts(text)) verify_out << count << '\n';
}
