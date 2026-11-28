// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Persistent_Binary_Trie.cpp"

#include <iostream>

int main()
{
    sirkit::PersistentBinaryTrie<unsigned, 9> trie(3);
    const auto first = trie.insert({}, 2);
    const auto second = trie.insert(first, 5);
    const auto third = trie.insert(second, 3);
    std::cout << *trie.kth_xor(first, third, 0) << '\n';    // 3
    std::cout << *trie.kth_xor(first, third, 0, 6) << '\n'; // 3 (= 5 ^ 6)
    std::cout << trie.size(first) << '\n';                  // 1
}
