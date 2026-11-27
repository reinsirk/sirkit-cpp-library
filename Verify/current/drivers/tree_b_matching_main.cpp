// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<int> cap(n);
    for (int &value : cap)
    {
        verify_in >> value;
    }
    std::vector<TreeBMatchingEdge> edges(n - 1);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
        --from;
        --to;
    }
    verify_out << maximum_weight_tree_b_matching(cap, edges).weight << '\n';
}
