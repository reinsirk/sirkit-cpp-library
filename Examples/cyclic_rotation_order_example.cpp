// SPDX-License-Identifier: CC0-1.0
#include "../String/Cyclic_Rotation_Order.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<int> ranks{1, 0, 1, 0};
    const std::vector<int> order{1, 3, 0, 2};
    assert(sirkit::cyclic_rotation_ranks("baba") == ranks);
    assert(sirkit::cyclic_rotation_order("baba") == order);
    assert(sirkit::count_cyclic_rotation_pairs("adb", "cab") == 4);
}
