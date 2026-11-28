// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<int> parent(n, -1), depth(n);
    for (int v = 1; v < n; ++v)
    {
        verify_in >> parent[v];
        --parent[v];
        depth[v] = 1 + depth[parent[v]];
    }
    int q{};
    verify_in >> q;
    std::vector<std::vector<std::pair<int, int>>> queries(n);
    for (int i = 0; i < q; ++i)
    {
        int vertex{}, target_depth{};
        verify_in >> vertex >> target_depth;
        queries[vertex - 1].emplace_back(target_depth, i);
    }
    const sirkit::DsuOnTree tree(parent);
    std::vector<int> counts(n), answers(q);
    tree.run(
        [&](int v) { ++counts[depth[v]]; },
        [&](int v) { --counts[depth[v]]; },
        [&](int v)
        {
            for (const auto &[d, id] : queries[v])
            {
                answers[id] = counts[d];
            }
        });
    for (const int answer : answers)
    {
        verify_out << answer << '\n';
    }
}
