// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Hamiltonian_Route.cpp"

#include <array>
#include <cstddef>
#include <iostream>
#include <optional>

int main()
{
    const auto route = sirkit::shortest_hamiltonian_cycle(3, 0, [](std::size_t u, std::size_t v) -> std::optional<int>
                                                          {
                                                              constexpr std::array next{2u, 0u, 1u};
                                                              if (v == next[u])
                                                              {
                                                                  return 10;
                                                              }
                                                              return std::nullopt;
                                                          });
    if (route)
    {
        std::cout << route->cost << '\n'; // 30
        for (const auto v : route->vertices)
        {
            std::cout << v << '\n'; // 0, 2, 1, 0
        }
    }
}
