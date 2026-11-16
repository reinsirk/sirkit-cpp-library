// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

int main() {
    std::string text; int count{}; verify_in >> text >> count;
    HashSet<std::string> patterns(count);
    std::size_t max_length = 0;
    while (count--) {
        std::string word; verify_in >> word;
        max_length = std::max(max_length, word.size());
        patterns.insert(std::move(word));
    }
    std::int64_t answer = 0;
    const std::string_view view(text);
    for (std::size_t i = 0; i < text.size(); ++i)
        for (std::size_t length = 1; length <= std::min(max_length, text.size() - i); ++length)
            answer += patterns.contains(std::string(view.substr(i, length)));
    verify_out << answer << '\n';
}
