// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Coordinate_Compression.cpp"

#include <cassert>
#include <vector>

int main()
{
    const sirkit::CoordinateCompression coordinates(std::vector{100, -5, 100, 42});
    assert(coordinates.size() == 3);
    assert(coordinates.find(100) == 2);
    assert(coordinates.lower_bound(50) == 2);
    assert(!coordinates.find(50));
    assert(coordinates.value(1) == 42);
}
