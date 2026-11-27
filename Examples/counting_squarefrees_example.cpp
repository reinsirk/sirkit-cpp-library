// SPDX-License-Identifier: CC0-1.0
#include "../Math/Counting_Squarefrees.cpp"
#include <cassert>

int main()
{
    assert(sirkit::count_squarefrees(10) == 7);
    assert(sirkit::count_squarefrees(1) == 1);
}
