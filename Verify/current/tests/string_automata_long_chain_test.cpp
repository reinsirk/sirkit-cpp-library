// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#ifndef SUFFIX_AUTOMATON_SOURCE
#define SUFFIX_AUTOMATON_SOURCE "../../../String/Suffix_Automaton.cpp"
#endif
#ifndef PALINDROMIC_TREE_SOURCE
#define PALINDROMIC_TREE_SOURCE "../../../String/Palindromic_Tree.cpp"
#endif
#include SUFFIX_AUTOMATON_SOURCE
#include PALINDROMIC_TREE_SOURCE
int main(){
 constexpr std::size_t n=1000000;const std::string text(n,'a'),pattern(n/2,'a');
 {SuffixAutomaton<>sam(text);assert(sam.size()==n&&sam.state_count()==n+1&&sam.distinct_substring_count()==n&&sam.occurrences(pattern)==n-n/2+1);auto best=sam.longest_common_substring("baaaab");assert(best.length==4&&best.second_start==1);sam.append(static_cast<std::uint8_t>('b'));assert(sam.size()==n+1&&sam.occurrences(pattern)==n-n/2+1&&sam.distinct_substring_count()==2*n+1);sam.reset("aba");assert(sam.distinct_substring_count()==5);sam.clear();assert(sam.empty());}
 {PalindromicTree<>tree(text);assert(tree.size()==n&&tree.distinct_palindrome_count()==n&&tree.longest_suffix_length()==n&&tree.last_node()==static_cast<std::int32_t>(n));assert(tree.occurrences(pattern)==n-n/2+1&&tree.node_info(static_cast<std::int32_t>(n)).occurrences==1);assert(tree.parent(static_cast<std::int32_t>(n))==static_cast<std::int32_t>(n-2)&&tree.suffix_link(static_cast<std::int32_t>(n))==static_cast<std::int32_t>(n-1));tree.append(static_cast<std::uint8_t>('b'));assert(tree.longest_suffix_length()==1&&tree.distinct_palindrome_count()==n+1&&tree.longest_palindrome().length==n&&tree.occurrences(pattern)==n-n/2+1);tree.reset("abba");assert(tree.longest_suffix_length()==4);tree.clear();assert(tree.empty());}
 // 長い周期列も構築し、cloneと長い接尾辞リンクを使用する。
 std::string periodic;periodic.reserve(200000);for(int i=0;i<50000;++i)periodic+="abac";
 SuffixAutomaton<>sam(periodic);PalindromicTree<>tree(periodic);assert(sam.occurrences("abac")==50000&&tree.occurrences("aba")==50000&&tree.occurrences("cac")==0);
 std::cout<<"million chain and periodic200000 iterative construction/counts OK\n";
}
