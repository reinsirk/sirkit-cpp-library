// SPDX-License-Identifier: CC0-1.0
#include "../String/Lexicographic_String_Shuffle.cpp"

#include <array>
#include <iostream>
#include <string_view>

int main()
{
    const std::array<std::string_view, 3> words{"a", "aaz", "ab"};
    std::cout << sirkit::lexicographic_string_shuffle(words) << '\n'; // aaaabz
}
