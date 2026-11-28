// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

int main()
{
    std::uint64_t length{};
    std::size_t count{};
    verify_in >> length >> count;
    std::vector<std::string> words(count);
    for (auto &word : words)
    {
        verify_in >> word;
    }
    const std::vector<std::string_view> forbidden(words.begin(), words.end());
    verify_out << sirkit::count_strings_avoiding_patterns<2>(length, forbidden) << '\n';
}
