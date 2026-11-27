// SPDX-License-Identifier: CC0-1.0
#include "../String/Edit_Distance.cpp"
#include <cassert>

int main()
{
    assert(sirkit::edit_distance("kitten", "sitting") == 3);
    assert(!sirkit::edit_distance_at_most("kitten", "sitting", 2));
}
