// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Monotone_Line_Container.cpp"

#include <cassert>

int main()
{
    sirkit::MonotoneMinLineContainer hull;
    hull.add(3, 1);
    hull.add(1, 5);
    assert(hull.query_monotone(0)->value == 1);
    assert(hull.query_monotone(3)->value == 8);
    assert(hull.query(1)->slope == 3);
}
