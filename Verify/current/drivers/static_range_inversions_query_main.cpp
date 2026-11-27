// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    int n, q;
    verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    std::vector<std::pair<std::size_t, std::size_t>> queries(q);
    for (auto &[l, r] : queries)
    {
        verify_in >> l >> r;
    }
    for (auto value : range_inversion_counts(values, queries))
    {
        verify_out << value << '\n';
    }
}
