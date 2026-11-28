// SPDX-License-Identifier: CC0-1.0
#include <string>
#include <string_view>
#include <vector>
#include "Verify/current/common/fast_io.hpp"

int main()
{
    int tests = 0;
    verify_in >> tests;
    while (tests--)
    {
        int n = 0;
        verify_in >> n;
        std::vector<std::string> words(n);
        std::vector<std::string_view> views;
        views.reserve(n);
        for (auto &word : words)
        {
            verify_in >> word;
            views.push_back(word);
        }
        verify_out << *second_lexicographic_concatenation(views) << '\n';
    }
}
