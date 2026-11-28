// SPDX-License-Identifier: CC0-1.0
#include "../String/Pattern_Erasure.cpp"

#include <cassert>
#include <string>

int main()
{
    assert(sirkit::erase_pattern("ababa", "aba") == "ba");
    std::string text = "AABCBCZ";
    const auto removed = sirkit::erase_pattern_inplace(text, "ABC");
    assert(text == "Z" && removed == 2);
}
