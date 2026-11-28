// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tournament_Path.cpp"

#include <cassert>

int main()
{
    const auto path = sirkit::tournament_path(3, [](int u, int v)
                                              {
                                                  return (u + 1) % 3 == v;
                                              });
    assert(path.size() == 3);
    assert((path[0] + 1) % 3 == path[1]);
    assert((path[1] + 1) % 3 == path[2]);
}
