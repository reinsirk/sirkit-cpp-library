// SPDX-License-Identifier: CC0-1.0
#include "../String/Lexicographic_Concatenation.cpp"

#include <array>
#include <iostream>
#include <string_view>

int main()
{
    const std::array<std::string_view, 3> words{"b", "aab", "a"};
    const auto order = sirkit::lexicographic_concatenation_order(words);
    for (const auto i : order)
    {
        std::cout << words[i];
    }
    std::cout << '\n'
              << *sirkit::second_lexicographic_concatenation(words) << '\n';
}
