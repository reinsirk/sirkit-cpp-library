// SPDX-License-Identifier: CC0-1.0
#include "../String/Manacher.cpp"
#include <iostream>
#include <string>

using namespace sirkit;
int main() {
    const std::string text = "abacaba";
    const PalindromeRadii radii(text);
    const auto [left, right] = radii.longest();
    std::cout << text.substr(left, right - left) << '\n';
    std::cout << radii.is_palindrome(1, 6) << ' ' << radii.count() << '\n';
    for (int length : PalindromicLengths(text)) std::cout << length << ' ';
    std::cout << '\n';
}
