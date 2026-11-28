// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<int> preorder(n), inorder(n);
    for (auto *order : {&preorder, &inorder})
    {
        for (int &v : *order)
        {
            verify_in >> v;
            --v;
        }
    }
    const auto tree = sirkit::binary_tree_from_preorder_inorder(preorder, inorder);
    if (!tree || tree->root != 0)
    {
        verify_out << -1 << '\n';
        return 0;
    }
    for (const auto &[left, right] : tree->children)
    {
        verify_out << left + 1 << ' ' << right + 1 << '\n';
    }
}
