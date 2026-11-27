// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Complement_Components.cpp"
#include <cassert>

int main()
{
    const std::vector<std::pair<int, int>> edges{{0, 1}};
    const auto components = sirkit::complement_components(3, edges);
    assert(components.size() == 1 && components[0].size() == 3);
}
