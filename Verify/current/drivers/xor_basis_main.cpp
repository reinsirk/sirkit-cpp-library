// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
int main() {
    std::size_t n{}; verify_in >> n;
    std::vector<std::uint64_t> values(n);
    std::uint64_t total = 0;
    for (auto& value : values) { verify_in >> value; total ^= value; }
    XorBasis basis;
    for (auto value : values) basis.insert(value & ~total);
    verify_out << total + 2 * basis.max_xor() << '\n';
}
