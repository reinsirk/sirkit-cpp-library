#include "../Graph/Floyd-Warshall.cpp"
#include <cstdint>
#include <iostream>
#include <vector>

using namespace sirkit;

int main() {
    const std::int64_t inf = 1'000'000'000'000; // 省略時はkInf。
    std::vector initial(5, std::vector<std::int64_t>(5, inf));
    for (int i = 0; i < 5; ++i) initial[i][i] = 0;
    initial[0][1] = 4;
    initial[1][2] = -2;
    initial[0][2] = 10;
    initial[2][3] = 7;
    auto distance = initial;
    std::vector<std::vector<int>> next;
    std::cout << FloydWarshall(distance, next, inf) << '\n'; // 0
    std::cout << distance[0][2] << ' ' << (distance[0][4] == inf) << '\n'; // 2 1
    for (int vertex : FloydWarshallPath(next, 0, 2)) std::cout << vertex << ' ';
    std::cout << '\n'; // 0 1 2

    auto negative = initial;
    negative[2][1] = 1; // 1→2→1が負閉路。
    std::cout << FloydWarshall(negative, inf) << '\n'; // 1
    std::cout << (negative[0][3] == -inf) << ' ' << (negative[4][0] == inf)
              << ' ' << negative[4][4] << '\n'; // 1 1 0
}
