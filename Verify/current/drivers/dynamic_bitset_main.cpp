// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <string>
#include <vector>
int main() {
    std::size_t n{}, m{}, k{}; verify_in >> n >> m >> k;
    std::vector<DynamicBitset> rows, columns;
    rows.reserve(n); columns.reserve(k);
    for (std::size_t i = 0; i < n; ++i) {
        std::string row; verify_in >> row;
        rows.emplace_back(row);
    }
    for (std::size_t j = 0; j < k; ++j) columns.emplace_back(m);
    for (std::size_t i = 0; i < m; ++i) {
        std::string row; verify_in >> row;
        for (std::size_t j = 0; j < k; ++j) if (row[j] == '1') columns[j].set(m - 1 - i);
    }
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < k; ++j) verify_out << (rows[i].intersection_parity(columns[j]) ? '1' : '0');
        verify_out << '\n';
    }
}
