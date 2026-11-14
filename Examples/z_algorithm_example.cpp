// SPDX-License-Identifier: CC0-1.0
#include "../String/Z_Algorithm.cpp"
#include <iostream>
#include <string>

using namespace sirkit;
int main() {
    const std::string text = "ababa", pattern = "aba";
    for (int length : z_algorithm(text)) std::cout << length << ' ';
    std::cout << '\n';
    for (int position : ZMatches(pattern, text)) std::cout << position << ' ';
    std::cout << '\n';
    const auto lengths = PrefixMatches(pattern, text);
    std::cout << lengths[2] << '\n';
}
