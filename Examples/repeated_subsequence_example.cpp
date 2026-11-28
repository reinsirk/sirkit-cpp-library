// SPDX-License-Identifier: CC0-1.0
#include "../String/Repeated_Subsequence.cpp"

#include <cassert>

int main()
{
    const sirkit::RepeatedSubsequenceIndex index("abc");
    assert(index.contains_expanded(3, "ab", 2));
    assert(!index.contains_expanded(3, "ab", 3));
    assert(index.max_repetition(3, "ab") == 2);
    assert(index.max_repetition(3, "ab", 1) == 1);
}
