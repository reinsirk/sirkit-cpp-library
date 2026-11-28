// SPDX-License-Identifier: CC0-1.0
#include "../Math/Todd_Coxeter.cpp"

#include <array>
#include <cassert>
#include <iostream>

int main()
{
    const std::array<sirkit::MonoidRelation, 3> relations{{{{0, 0}, {}}, {{1, 1}, {}}, {{0, 1, 0}, {1, 0, 1}}}};
    const auto graph = sirkit::todd_coxeter(2, relations, 100);
    assert(graph);
    std::cout << graph->size() << '\n';                                                                  // 6
    std::cout << (graph->evaluate(std::array{0, 1, 0}) == graph->evaluate(std::array{1, 0, 1})) << '\n'; // 1
}
