// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n{}, q{};
    verify_in >> n >> q;
    std::vector<std::int64_t> weights(n);
    std::vector<int> colors(n);
    for (auto &weight : weights)
    {
        verify_in >> weight;
    }
    for (auto &color : colors)
    {
        verify_in >> color;
    }
    std::vector<sirkit::DynamicColorComponentSum::Edge> edges(n - 1);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    sirkit::DynamicColorComponentSum tree(weights, colors, edges);
    for (int i = 0; i < q; ++i)
    {
        int kind{}, vertex{};
        verify_in >> kind >> vertex;
        --vertex;
        if (kind == 1)
        {
            tree.toggle(vertex);
        }
        else if (kind == 2)
        {
            std::int64_t delta{};
            verify_in >> delta;
            tree.add_weight(vertex, delta);
        }
        else
        {
            verify_out << tree.component_sum(vertex) << '\n';
        }
    }
}
