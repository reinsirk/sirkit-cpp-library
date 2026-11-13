// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
int main() {
    std::int32_t n{},m{}; verify_in >> n >> m;
    GeneralMatching graph(n);
    for (std::int32_t i=0; i<m; ++i) {
        std::int32_t u{},v{}; verify_in >> u >> v; graph.add_edge(u,v);
    }
    verify_out << graph.solve() << '\n';
    for (const auto& [u,v] : graph.pairs()) verify_out << u << ' ' << v << '\n';
}
