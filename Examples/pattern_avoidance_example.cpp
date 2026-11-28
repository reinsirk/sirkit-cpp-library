// SPDX-License-Identifier: CC0-1.0
#include "../String/Pattern_Avoidance.cpp"

#include <array>
#include <cassert>
#include <string_view>

int main()
{
    const std::array<std::string_view, 1> forbidden{"aa"};
    assert(sirkit::count_strings_avoiding_patterns<2>(3, forbidden) == 5);
    assert(sirkit::count_strings_avoiding_patterns<2>(20, forbidden) == 17711);
}
