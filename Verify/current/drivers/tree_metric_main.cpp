// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<std::size_t> row(n);
    for (int u = 1; u < n; ++u)
    {
        row[u] = row[u - 1] + static_cast<std::size_t>(n - u);
    }
    std::vector<std::uint16_t> values(row.back());
    for (auto &d : values)
    {
        verify_in >> d;
    }
    const auto distance = [&](int u, int v)
    {
        if (u == v)
        {
            return std::uint16_t{0};
        }
        if (u > v)
        {
            std::swap(u, v);
        }
        return values[row[u] + static_cast<std::size_t>(v - u - 1)];
    };
    verify_out << (sirkit::reconstruct_tree_metric(n, distance) ? "Yes\n" : "No\n");
}
