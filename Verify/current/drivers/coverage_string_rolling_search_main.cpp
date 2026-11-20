// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_set>

// 大文字・長さ10以下の同じ長さの語なら、底27のハッシュは衝突しない。
// 26文字の差は25以下で、各窓の多項式値も2^61-1より小さい。
int main() {
    std::string text; int count{}; verify_in >> text >> count;
    const RollingHash text_hash(text, 27);
    std::array<std::unordered_set<std::uint64_t>, 11> patterns;
    while (count--) {
        std::string word; verify_in >> word;
        const RollingHash pattern_hash(word, 27);
        patterns[word.size()].insert(pattern_hash.whole());
    }
    std::uint64_t answer = 0;
    for (std::size_t length = 1; length <= 10 && length <= text.size(); ++length) {
        if (patterns[length].empty()) continue;
        for (std::size_t i = 0; i <= text.size() - length; ++i)
            answer += patterns[length].contains(text_hash.get(i, i + length));
    }
    verify_out << answer << '\n';
}
