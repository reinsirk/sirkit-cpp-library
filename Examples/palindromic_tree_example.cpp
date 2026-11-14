// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <iostream>
#include "../String/Palindromic_Tree.cpp"

using namespace sirkit;
int main(){
 PalindromicTree<>tree("ababa");std::cout<<tree.distinct_palindrome_count()<<' '<<tree.longest_suffix_length()<<' '<<tree.occurrences("aba")<<'\n'; // 5 5 2
 auto longest=tree.longest_palindrome();std::cout<<longest.start<<' '<<longest.length<<'\n'; // 0 5
 for(auto info:tree.palindromes())std::cout<<tree.text().substr(info.start,info.length)<<':'<<info.occurrences<<' ';std::cout<<'\n'; // a:3 b:2 aba:2 bab:1 ababa:1
 auto id=tree.append(static_cast<std::uint8_t>('b'));std::cout<<id<<' '<<tree.parent(id)<<' '<<tree.suffix_link(id)<<'\n'; // 6 3 4
 auto copy=tree;copy.append(copy.text().substr(0,2));std::cout<<tree.text()<<' '<<copy.text()<<'\n'; // ababab abababab
 tree.reset("abba");std::cout<<tree.contains("bb")<<' '<<tree.occurrences("")<<'\n'; // 1 5
 tree.clear();std::cout<<tree.empty()<<' '<<tree.last_node()<<'\n'; // 1 0
}
