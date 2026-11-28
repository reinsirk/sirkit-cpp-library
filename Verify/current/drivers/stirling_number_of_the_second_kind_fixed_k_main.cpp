// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>

int main()
{
    std::size_t n{}, k{};
    verify_in >> n >> k;
    const auto column = sirkit::stirling_second_column(n, k);
    for (std::size_t i = 0; i < column.size(); ++i)
    {
        verify_out << column[i] << (i + 1 == column.size() ? '\n' : ' ');
    }
}
