// SPDX-License-Identifier: CC0-1.0
#include <cstddef>
#include <cstdint>
#include <vector>
#include "Verify/current/common/fast_io.hpp"

int main() {
    int n{};
    std::uint64_t k{};
    if (!(verify_in >> n)) return 0;
    verify_in >> k;
    std::vector<int> next(static_cast<std::size_t>(n)), values(static_cast<std::size_t>(n));
    for (auto& v : next) { verify_in >> v; --v; }
    for (auto& value : values) verify_in >> value;
    FunctionalGraph graph(next);
    for (int v = 0; v < n; ++v) {
        if (v) verify_out << ' ';
        verify_out << values[static_cast<std::size_t>(graph.kth_successor(v, k))];
    }
    verify_out << '\n';
}
