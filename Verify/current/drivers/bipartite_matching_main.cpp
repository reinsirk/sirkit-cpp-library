// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
int main() {
    int left{},right{},edges{}; verify_in >> left >> right >> edges;
    BipartiteMatching graph(left,right);
    for (int i=0;i<edges;++i) { int u{},v{}; verify_in >> u >> v; graph.add_edge(u,v); }
    verify_out << graph.solve() << '\n';
    for (auto [u,v] : graph.pairs()) verify_out << u << ' ' << v << '\n';
}
