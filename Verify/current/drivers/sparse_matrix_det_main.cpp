// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <random>
#include <vector>

int main()
{
    std::size_t n{}, k{};
    verify_in >> n >> k;
    std::vector<sirkit::SparseDeterminantEntry> entries(k);
    for (auto &[row, column, value] : entries)
    {
        verify_in >> row >> column >> value;
    }
    const auto result = sirkit::sparse_determinant_mod998244353(n, entries, std::random_device{}(), 8);
    verify_out << result.value() << '\n';
}
