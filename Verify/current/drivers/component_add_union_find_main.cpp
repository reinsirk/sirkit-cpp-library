// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

int main()
{
    int n{}, q{};
    verify_in >> n >> q;
    sirkit::ComponentAddUnionFind<> values(n);
    for (int i = 0; i < q; ++i)
    {
        int type{}, a{}, b{};
        verify_in >> type >> a >> b;
        --a;
        if (type == 1)
        {
            values.unite(a, b - 1);
        }
        else if (type == 2)
        {
            values.add_component(a, b);
        }
        else
        {
            verify_out << values.get(a) << '\n';
        }
    }
}
