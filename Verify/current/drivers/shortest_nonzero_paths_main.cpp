// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

int main()
{
    int n{}, m{}, bits{};
    verify_in >> n >> m >> bits;
    std::vector<sirkit::GroupPathEdge<std::uint32_t>> edges(m);
    std::string binary;
    binary.reserve(bits);
    for (auto &[from, to, length, label] : edges)
    {
        verify_in >> from >> to >> length >> binary;
        --from;
        --to;
        for (const char bit : binary)
        {
            label = (label << 1) | static_cast<std::uint32_t>(bit - '0');
        }
    }
    const auto answer = sirkit::shortest_nonzero_paths(n, edges, n - 1, std::uint32_t{0}, std::bit_xor<>{}, std::identity{});
    for (int v = 0; v + 1 < n; ++v)
    {
        if (answer[v])
        {
            verify_out << *answer[v] << '\n';
        }
        else
        {
            verify_out << -1 << '\n';
        }
    }
}
