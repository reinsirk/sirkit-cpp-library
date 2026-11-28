// SPDX-License-Identifier: CC0-1.0
#include <iostream>
#include <string>
#include <vector>

int main()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    std::cin >> n;
    std::vector<std::string> outgoing(n);
    for (auto &row : outgoing)
    {
        std::cin >> row;
    }
    const auto path = tournament_path(n, [&](int from, int to)
                                      {
                                          return outgoing[from][to] == '1';
                                      });
    for (int vertex : path)
    {
        std::cout << vertex << '\n';
    }
}
