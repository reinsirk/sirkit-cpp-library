// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Range_Inversions.cpp"
#include <cassert>

int main()
{
    const std::vector values{4, 1, 4, 0};
    assert(sirkit::inversion_count(values) == 4);
}
