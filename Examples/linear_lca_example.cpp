// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Linear_LCA.cpp"

#include <array>
#include <iostream>

int main()
{
    const std::array parent{-1, 0, 0, 1, 1, -1};
    const sirkit::LinearLCA lca(parent);
    std::cout << lca.get(3, 4) << '\n';
    std::cout << lca.get_distance(2, 4) << '\n';
    std::cout << lca.get(3, 5) << '\n';
}
