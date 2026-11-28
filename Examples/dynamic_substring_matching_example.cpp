// SPDX-License-Identifier: CC0-1.0
#include "../String/Dynamic_Substring_Matching.cpp"
#include <cassert>

int main()
{
    sirkit::DynamicSubstringMatching text("ababa");
    assert(text.count("aba") == 2);
    text.set(2, 'x');
    assert(!text.contains("aba"));
}
