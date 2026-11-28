// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<int> values(n);
    for (int v = 1; v < n; ++v)
    {
        verify_in >> values[v];
    }
    std::vector<std::vector<int>> graph(n);
    for (int i = 1; i < n; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        --u;
        --v;
        graph[u].push_back(v);
        graph[v].push_back(u);
    }
    std::vector<int> parent(n, -1), order{0};
    order.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        const int u = order[i];
        for (const int v : graph[u])
        {
            if (v != parent[u])
            {
                parent[v] = u;
                order.push_back(v);
            }
        }
    }

    std::vector<sirkit::MeldableHeap<int>> heaps(n);
    for (auto at = order.rbegin(); at != order.rend(); ++at)
    {
        const int v = *at;
        if (!heaps[v].empty())
        {
            heaps[v].pop();
        }
        if (v != 0)
        {
            heaps[v].push(values[v]);
            heaps[parent[v]].meld(heaps[v]);
        }
    }
    verify_out << (heaps[0].empty() ? 0 : heaps[0].top()) << '\n';
}
