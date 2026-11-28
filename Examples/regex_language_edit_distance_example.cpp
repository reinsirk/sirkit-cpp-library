// SPDX-License-Identifier: CC0-1.0
#include "../String/Regex_Language_Edit_Distance.cpp"

#include <cassert>

int main()
{
    assert(sirkit::regex_language_edit_distance("a*bcd", "aaaabd?") == 1);
    assert(sirkit::regex_language_edit_distance("a*b?", "bbb") == 2);
}
