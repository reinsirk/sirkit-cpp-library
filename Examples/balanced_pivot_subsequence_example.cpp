// SPDX-License-Identifier: CC0-1.0
#include "../String/Balanced_Pivot_Subsequence.cpp"

#include <cassert>

int main()
{
    const sirkit::BalancedPivotSubsequenceIndex index("a/a/bb", 'a', '/', 'b');
    const auto answer = index.query(0, 6);
    assert(answer && answer->pivot == 3 && answer->length() == 5);
    const auto tie = index.query(0, 5);
    assert(tie && tie->pivot == 1 && tie->pairs == 1);
    assert(!index.query(4, 6));
}
