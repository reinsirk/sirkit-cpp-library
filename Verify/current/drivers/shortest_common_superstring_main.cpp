// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

int main()
{
    std::size_t n = 0;
    verify_in >> n;
    std::vector<std::string> words(n);
    std::vector<std::string_view> views;
    views.reserve(n);
    for (auto &word : words)
    {
        verify_in >> word;
        views.push_back(word);
    }
    verify_out << sirkit::shortest_common_superstring(views).size() << '\n';
}
