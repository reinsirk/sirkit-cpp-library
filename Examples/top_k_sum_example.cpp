// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Top_K_Sum.cpp"

#include <cassert>
#include <cstdint>

int main()
{
    sirkit::TopKSum<int, true, std::int64_t> largest(2);
    for (int value : {3, 7, 5, 7})
    {
        largest.insert(value);
    }
    assert(largest.sum() == 14);
    largest.erase(7);
    assert(largest.sum() == 12);
}
