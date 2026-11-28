// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Offline_DAG_Reachability.cpp"

#include <array>
#include <iostream>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 3}};
    const std::array queries{std::pair{0, 3}, std::pair{1, 2}, std::pair{3, 0}};
    for (const bool reachable : sirkit::offline_dag_reachability(4, edges, queries))
    {
        std::cout << (reachable ? "Yes\n" : "No\n");
    }
}
