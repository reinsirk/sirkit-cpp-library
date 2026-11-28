// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Offline_Li_Chao_Tree.cpp"

#include <cassert>

int main()
{
    sirkit::OfflineLiChaoTree<int> lines;
    const auto id = lines.add_line({2, 1});
    lines.add_segment(0, 2, {0, 4});
    lines.query(1);
    lines.erase(id);
    lines.query(1);
    lines.query(2);
    const auto answer = lines.solve();
    assert(answer[0] == 3 && answer[1] == 4 && !answer[2]);
}
