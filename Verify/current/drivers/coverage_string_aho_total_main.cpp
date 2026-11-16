// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>

int main() {
    std::string text; int count{}; verify_in >> text >> count;
    AhoCorasick<26, 'A'> patterns;
    while (count--) {
        std::string word; verify_in >> word;
        patterns.add(word);
    }
    patterns.build();
    verify_out << patterns.count_total(text) << '\n';
}
