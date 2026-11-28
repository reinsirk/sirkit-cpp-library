// SPDX-License-Identifier: CC0-1.0
#include "../String/Substring_Comparison.cpp"
#include <cassert>
#include <compare>

int main()
{
    const sirkit::SubstringComparison index("banana");
    assert(index.common_prefix(1, 3) == 3);
    assert(index.equal(1, 3, 3));
    assert(index.compare(0, 3, 1, 3) == std::strong_ordering::greater);
}
