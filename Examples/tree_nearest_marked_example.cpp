// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Nearest_Marked.cpp"

#include <iostream>

int main()
{
    sirkit::TreeNearestMarked tree({{1}, {0, 2}, {1}});
    tree.set_marked(2);
    const auto nearest = tree.nearest(0);
    if (nearest)
    {
        std::cout << nearest->vertex << ' ' << nearest->distance << '\n';
    }
    tree.toggle(2);
}
