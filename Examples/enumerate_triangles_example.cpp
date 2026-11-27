// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Enumerate_Triangles.cpp"
#include <cassert>

int main()
{
    const std::vector<std::pair<int, int>> edges{{0, 1}, {0, 2}, {1, 2}};
    int count = 0;
    sirkit::enumerate_triangles(3, edges, [&](int, int, int) { ++count; });
    assert(count == 1);
}
