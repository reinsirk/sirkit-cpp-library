// SPDX-License-Identifier: CC0-1.0
#include "../String/Prefix_Function.cpp"
#include <cassert>

int main()
{
    const auto matches = sirkit::find_occurrences("ababa", "aba");
    assert((matches == std::vector<std::size_t>{0, 2}));
}
