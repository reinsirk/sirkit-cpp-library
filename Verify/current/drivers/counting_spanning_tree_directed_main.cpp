// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <atcoder/modint>
#include <vector>

int main()
{
    using Mint = atcoder::modint998244353;
    int n, m, root;
    verify_in >> n >> m >> root;
    std::vector<SpanningTreeCountEdge<Mint>> edges(m);
    for (auto &edge : edges)
    {
        verify_in >> edge.from >> edge.to;
    }
    verify_out << count_directed_spanning_trees<Mint>(n, root, edges).val() << '\n';
}
