// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Kruskal_Reconstruction_Tree.cpp"

#include <array>
#include <cassert>

int main()
{
    using Edge = sirkit::SpanningTreeEdge;
    const std::array edges{Edge{0, 1, 3}, Edge{1, 2, 4}, Edge{0, 2, 9}};
    const sirkit::KruskalReconstructionTree tree(4, edges);
    assert(tree.merge_weight(0, 2) == 4);
    assert(tree.node(tree.component_at(0, 3)).vertex_count == 2);
    assert(!tree.merge_weight(0, 3));
}
