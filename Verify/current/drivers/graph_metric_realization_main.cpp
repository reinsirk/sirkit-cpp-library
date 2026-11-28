// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<std::uint32_t> values(static_cast<std::size_t>(n) * n);
    for (auto &d : values)
    {
        verify_in >> d;
    }
    const auto graph = sirkit::realize_graph_metric(n, [&](int u, int v)
                                                    {
                                                        return values[static_cast<std::size_t>(u) * n + v];
                                                    });
    if (graph)
    {
        verify_out << graph->total_weight.value() << '\n';
    }
    else
    {
        verify_out << -1 << '\n';
    }
}
