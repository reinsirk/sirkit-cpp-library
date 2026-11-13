// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
int main() {
    std::int32_t n{}; verify_in >> n;
    std::vector<std::int32_t> values(static_cast<std::size_t>(n));
    for (auto& value : values) verify_in >> value;
    CartesianTree tree(values);
    for (std::int32_t i = 0; i < n; ++i)
        verify_out << (i == tree.root() ? i : tree.parent(i)) << (i + 1 == n ? '\n' : ' ');
}
