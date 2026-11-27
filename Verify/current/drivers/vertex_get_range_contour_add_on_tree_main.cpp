// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n, q;
    verify_in >> n >> q;
    std::vector<std::int64_t> initial(n);
    for (auto &value : initial)
    {
        verify_in >> value;
    }
    std::vector<std::pair<int, int>> edges(n - 1);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
    }
    ContourAddVertexGet<> work(TreeContourIndex(n, edges), initial);
    for (int i = 0; i < q; ++i)
    {
        int type, v;
        verify_in >> type >> v;
        if (type == 0)
        {
            int l, r;
            std::int64_t delta = 0;
            verify_in >> l >> r >> delta;
            work.add_range(v, l, r, delta);
        }
        else
        {
            verify_out << work.get(v) << '\n';
        }
    }
}
