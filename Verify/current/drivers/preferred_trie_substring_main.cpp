// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>

int main() {
    std::string text; int m{}; verify_in >> text >> m;
    Trie patterns;
    std::size_t max_length = 0;
    for (int i = 0; i < m; ++i) {
        std::string word; verify_in >> word;
        patterns.insert(word);
        max_length = std::max(max_length, word.size());
    }
    std::int64_t answer = 0;
    const std::string_view view(text);
    for (std::size_t i = 0; i < text.size(); ++i)
        for (std::size_t length = 1; length <= std::min(max_length, text.size() - i); ++length)
            answer += static_cast<std::int64_t>(patterns.count(view.substr(i, length)));
    verify_out << answer << '\n';
}
