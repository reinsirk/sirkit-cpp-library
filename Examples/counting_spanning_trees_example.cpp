// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Counting_Spanning_Trees.cpp"

#include <array>
#include <atcoder/modint>
#include <cassert>

int main()
{
    using Mint = atcoder::modint998244353;
    using Edge = sirkit::SpanningTreeCountEdge<Mint>;
    const std::array edges{Edge{0, 1, 2}, Edge{1, 2, 3}, Edge{0, 2, 5}};
    assert(sirkit::count_undirected_spanning_trees<Mint>(3, edges).val() == 31);
    assert(sirkit::count_directed_spanning_trees<Mint>(3, 0, edges).val() == 16);
}
