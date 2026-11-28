// SPDX-License-Identifier: CC0-1.0
#include "../String/Sorted_Substring_Index.cpp"

#include <cassert>

int main()
{
    sirkit::SortedSubstringIndex<> index("zacef");
    assert(index.is_sorted_slice(1, 5));
    index.set(0, 'b');
    assert(index.is_nondecreasing(1, 5));
    assert(!index.is_sorted_slice(1, 5));
}
