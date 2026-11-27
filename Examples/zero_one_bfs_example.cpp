// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Zero_One_BFS.cpp"
#include <cassert>

int main()
{
    const auto result = sirkit::zero_one_bfs(2, 0, [](int v, auto visit)
                                             { if (v == 0) visit(1, 1); });
    assert(result.distance[1] == 1 && result.parent[1] == 0);
}
