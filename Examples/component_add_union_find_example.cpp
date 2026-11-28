// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Component_Add_Union_Find.cpp"

#include <cassert>

int main()
{
    sirkit::ComponentAddUnionFind<> values(3);
    values.add_component(0, 10);
    values.unite(0, 1);
    assert(values.get(1) == 0);

    values.add_component(1, 3);
    assert(values.get(0) == 13 && values.get(1) == 3);
    assert(values.component_sum(0) == 16);
    assert(!values.issame(0, 2));
}
