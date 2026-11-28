// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Dynamic_Edge_Products.cpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <utility>

int main()
{
    constexpr std::array edges{std::pair{0, 1}, std::pair{1, 2}};
    constexpr std::array<std::int64_t, 3> weights{2, 3, 5};
    sirkit::DynamicEdgeProducts<std::int64_t> products(3, edges, weights);
    products.set_active(0, true);
    products.set_active(1, true);
    products.add(1, 1);
    std::cout << products.neighbor_sum(1) << ' ' << products.sum_edge_products() << '\n'; // 7 28
}
