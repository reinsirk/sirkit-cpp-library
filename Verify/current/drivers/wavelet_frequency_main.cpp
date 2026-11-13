// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>

int main() {
    int n{}, q{}; verify_in >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto& value : values) verify_in >> value;
    WaveletMatrix<std::int64_t> matrix(values);
    while (q--) {
        int left{}, right{}; std::int64_t value{};
        verify_in >> left >> right >> value;
        verify_out << matrix.rank(value, left, right) << '\n';
    }
}
