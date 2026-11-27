// SPDX-License-Identifier: CC0-1.0
#include "../String/Prefix_Substring_LCS.cpp"
#include <cassert>

int main()
{
    const sirkit::PrefixSubstringLCS lcs("abc", "zbcab");
    assert(lcs.length(2, 1, 5) == 2);
    assert(lcs.length(3, 0, 3) == 2);
    assert(lcs.length(3, 1, 1) == 0);
}
