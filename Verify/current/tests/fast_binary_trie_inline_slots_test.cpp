// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef FAST_TRIE_SOURCE
#define FAST_TRIE_SOURCE "../../../Fast/GCC/DataStructure/Binary_Trie.cpp"
#endif
#include FAST_TRIE_SOURCE
#include FAST_TRIE_SOURCE
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <type_traits>
#include <vector>
namespace { std::uint64_t checks=0;void check(bool ok){++checks;if(!ok)std::abort();} }
int main(){
    {
        BinaryTrie<unsigned,0,1> empty;check(empty.empty());check(empty.count(0)==0);check(!empty.contains(1));check(empty.find(0).second.empty());check(empty.count_less(2)==0);check(empty.count_xor_less(1,2)==0);
    }
    {
        BinaryTrie<unsigned,0,2> single;
        for(int round=0;round<1000;++round){const unsigned key=round&1;single.add(key,3);single.add(key,4);check(single.count(key)==2);check(single.del(key,3));check(single.find(key).second==std::vector<int>{4});check(single.del(key,4));check(single.empty());}
    }
    {
        BinaryTrie<unsigned,0,3> both;both.add(0,1);both.add(1,2);check(both.size()==2);check(both.find(0).second==std::vector<int>{1});check(both.find(1).second==std::vector<int>{2});check(both.del(0,1));check(both.del(1,2));check(both.empty());
    }
    {
        BinaryTrie<std::uint32_t,7,1024> tree;
        for(int round=0;round<100;++round){
            for(int key=0;key<80;++key)for(int id=0;id<9;++id)tree.add(key,id);
            check(tree.size()==720);
            for(int key=0;key<80;++key){check(tree.find(key).second.size()==9);for(int id=0;id<9;++id)check(tree.del(key,id));}
            check(tree.empty());
        }
        static_assert(!std::is_copy_constructible_v<decltype(tree)>);
        static_assert(!std::is_move_constructible_v<decltype(tree)>);
    }
    std::printf("fast BinaryTrie initial-slot checks=%ju\n",static_cast<std::uintmax_t>(checks));
}
