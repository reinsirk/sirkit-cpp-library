// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{};
    std::uint64_t length{};
    verify_in >> n >> m >> length;
    std::vector<sirkit::ExactLengthWalkArc<int>> edges(m);
    for (int i = 0; i < m; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        edges[i] = {u - 1, v - 1, i + 1};
    }
    constexpr int kInf = 1 << 30;
    const auto times = sirkit::fold_exact_length_walks(n, edges, 0, length, kInf, 0, [](int a, int b)
                                                       {
                                                           return std::min(a, b);
                                                       },
                                                       [](int a, int b)
                                                       {
                                                           return std::max(a, b);
                                                       });
    for (int v = 0; v < n; ++v)
    {
        if (v != 0)
        {
            verify_out << ' ';
        }
        verify_out << (times[v] == kInf ? -1 : times[v]);
    }
    verify_out << '\n';
}
