// SPDX-License-Identifier: CC0-1.0
#include "../String/Suffix_Array.cpp"
#include <iostream>
#include <string>
int main() {
    const std::string text = "banana";
    const auto sa = suffix_array(text);
    for (int position : sa) std::cout << position << ' ';
    std::cout << '\n';
    for (int length : lcp_array(text, sa)) std::cout << length << ' ';
    std::cout << '\n';
    const auto [first, last] = SuffixArrayEqualRange(text, sa, "ana");
    for (int i = first; i < last; ++i) std::cout << sa[i] << ' ';
    std::cout << '\n';
    const std::vector<std::int64_t> integers{-7, 20, -7};
    const auto integer_sa = suffix_array(integers);
    const auto bounded_sa = suffix_array(std::vector<int>{0, 2, 0}, 2);
    std::cout << integer_sa[0] << ' ' << bounded_sa[0] << '\n';
    const std::vector<std::string> words{"cat", "dog", "cat"};
    const auto word_sa = suffix_array(words);
    std::cout << word_sa.front() << '\n';
}
