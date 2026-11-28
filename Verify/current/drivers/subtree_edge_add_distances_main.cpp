// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    using Tree = sirkit::SubtreeEdgeAddDistances<>;
    std::vector<Tree::Edge> edges(n - 1);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v >> weight;
    }
    Tree tree(n, edges);
    int q{};
    verify_in >> q;
    while (q-- > 0)
    {
        int type{}, v{};
        verify_in >> type >> v;
        if (type == 1)
        {
            std::int64_t delta{};
            verify_in >> delta;
            tree.add_subtree(v, delta);
        }
        else
        {
            verify_out << tree.root_distance(v) << '\n';
        }
    }
}
