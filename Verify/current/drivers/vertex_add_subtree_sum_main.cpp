// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<std::uint64_t> values(n);
    for (auto &x : values)
    {
        verify_in >> x;
    }
    std::vector<int> parents(n, -1);
    for (int v = 1; v < n; ++v)
    {
        verify_in >> parents[v];
    }
    const sirkit::SubtreeIntervals intervals(parents);
    std::vector<std::uint64_t> flat(n);
    for (int i = 0; i < n; ++i)
    {
        flat[i] = values[intervals.vertex_at(i)];
    }
    sirkit::fenwick_tree<std::uint64_t> sums(flat);
    for (int i = 0; i < q; ++i)
    {
        int type = 0, vertex = 0;
        verify_in >> type >> vertex;
        if (type == 0)
        {
            std::uint64_t delta = 0;
            verify_in >> delta;
            sums.add(intervals.index(vertex), delta);
        }
        else
        {
            const auto [left, right] = intervals.range(vertex);
            verify_out << sums.sum(left, right) << '\n';
        }
    }
}
