// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    int n{}, q{};
    verify_in >> n >> q;
    std::vector<int> label(n);
    for (auto &value : label)
    {
        verify_in >> value;
    }
    std::vector<std::pair<int, int>> edges(n - 1), queries(q), bounds(q);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    for (int i = 0; i < q; ++i)
    {
        auto &[u, v] = queries[i];
        auto &[lo, hi] = bounds[i];
        verify_in >> u >> v >> lo >> hi;
        --u;
        --v;
    }

    const sirkit::TreeMo tree(n, edges);
    std::vector<int> frequency(n + 1), at_least(n + 2), answers(q);
    const auto add = [&](int v)
    {
        ++at_least[++frequency[label[v]]];
    };
    const auto erase = [&](int v)
    {
        --at_least[frequency[label[v]]--];
    };
    tree.run(queries, add, erase, [&](std::size_t query)
             {
                 const auto [lo, hi] = bounds[query];
                 answers[query] = at_least[lo] - at_least[hi + 1];
             });
    for (const int answer : answers)
    {
        verify_out << answer << '\n';
    }
}
