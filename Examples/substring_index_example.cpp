// SPDX-License-Identifier: CC0-1.0
#include "../String/Substring_Index.cpp"
#include <cassert>

int main()
{
    const sirkit::SubstringIndex index("banana");
    assert(index.occurrences("ana").size() == 2);
    assert(index.kth_distinct_substring(3).value() == "ana");
    assert(index.distinct_substring_count() == 15);
}
