// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
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
    EulerTourTree<> forest(n, initial);
    for (int i = 1; i < n; ++i)
    {
        int u, v;
        verify_in >> u >> v;
        static_cast<void>(forest.link(u, v));
    }
    for (int i = 0; i < q; ++i)
    {
        int type;
        verify_in >> type;
        if (type == 0)
        {
            int u, v, w, x;
            verify_in >> u >> v >> w >> x;
            static_cast<void>(forest.cut(u, v));
            static_cast<void>(forest.link(w, x));
        }
        else if (type == 1)
        {
            int v, p;
            std::int64_t delta = 0;
            verify_in >> v >> p >> delta;
            static_cast<void>(forest.subtree_add(v, p, delta));
        }
        else
        {
            int v, p;
            verify_in >> v >> p;
            verify_out << *forest.subtree_sum(v, p) << '\n';
        }
    }
}
