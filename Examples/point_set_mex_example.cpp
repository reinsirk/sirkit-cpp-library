// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Point_Set_Mex.cpp"

#include <cassert>

int main()
{
    sirkit::PointSetMex<int> index({0, 0, 2, -1});
    assert(index.mex() == 1);
    index.set(1, 1);
    assert(index.mex() == 3);
    assert(index.get(3) == -1);
}
