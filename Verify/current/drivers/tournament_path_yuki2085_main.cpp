// SPDX-License-Identifier: CC0-1.0
#include <cstdlib>
#include <iostream>

int main()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    if (!(std::cin >> n))
    {
        return 0;
    }
    const auto path = tournament_path(n, [](int from, int to)
                                      {
                                          std::cout << "? " << from + 1 << ' ' << to + 1 << std::endl;
                                          int answer;
                                          if (!(std::cin >> answer) || answer < 0)
                                          {
                                              std::exit(0);
                                          }
                                          return answer == 1;
                                      });
    std::cout << "!\n"
              << n - 1 << '\n';
    for (int i = 0; i < n; ++i)
    {
        if (i != 0)
        {
            std::cout << ' ';
        }
        std::cout << path[i] + 1;
    }
    std::cout << std::endl;
}
