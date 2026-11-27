// SPDX-License-Identifier: CC0-1.0
#include "../String/Palindromic_Factorization.cpp"
#include <cassert>
#include <string_view>
#include <vector>

int main()
{
    const std::string_view text = "noonabbad";
    const auto cuts = sirkit::min_palindromic_partition(text);
    assert((cuts == std::vector<int>{0, 4, 8, 9}));
    assert(text.substr(cuts[1], cuts[2] - cuts[1]) == "abba");
}
