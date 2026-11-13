// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Pruefer_Code.cpp"
#include <iostream>
#include <vector>
int main() {
    std::vector<int> code{3, 4, 3, 5, 4};
    auto edges = pruefer_decode(7, code);
    for (const int vertex : pruefer_encode(7, edges)) std::cout << vertex << ' ';
    std::cout << '\n'; // 3 4 3 5 4
    for (const int degree : pruefer_degrees(7, code)) std::cout << degree << ' ';
    std::cout << '\n'; // 1 1 1 3 3 2 1
    std::cout << pruefer_decode(code).size() << '\n'; // 6
    std::vector<int> degrees{1, 2, 1};
    std::cout << pruefer_from_degrees(degrees)->size() << '\n'; // 1
    std::cout << tree_from_degrees(degrees)->size() << '\n'; // 2
    std::vector<int> wanted{1, 2, 1, 2, 2, 2};
    std::vector<std::pair<int,int>> forest{{1,2}, {0,3}};
    auto added = complete_tree_with_degrees(wanted, forest);
    std::cout << added->size() << '\n'; // 3
    std::cout << tree_from_degrees(std::vector<int>{2, 2, 2}).has_value() << '\n'; // 0
    std::cout << pruefer_decode(1, {}).size() << '\n'; // 0
}
