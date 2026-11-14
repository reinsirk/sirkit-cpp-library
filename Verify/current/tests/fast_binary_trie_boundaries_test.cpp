// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef FAST_TRIE_SOURCE
#define FAST_TRIE_SOURCE "../../../Fast/GCC/DataStructure/Binary_Trie.cpp"
#endif
#include FAST_TRIE_SOURCE
#include FAST_TRIE_SOURCE
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <map>
#include <random>
#include <type_traits>
#include <vector>
namespace {
std::uint64_t checks=0;
void check(bool ok){++checks;if(!ok)std::abort();}
template<class T,int Log>void test(std::uint64_t seed){
    using U=std::make_unsigned_t<T>;
    constexpr U mask=[] { if constexpr(Log+1==std::numeric_limits<U>::digits)return std::numeric_limits<U>::max();else return static_cast<U>((U{1}<<(Log+1))-1); }();
    BinaryTrie<T,Log,65536> trie;std::map<U,std::vector<int>> model;std::mt19937_64 random(seed);
    std::vector<U> keys{0,1,mask,static_cast<U>(mask/2),static_cast<U>(U{1}<<Log)};
    for(int i=0;i<120;++i)keys.push_back(static_cast<U>(random())&mask);
    for(std::size_t i=0;i<keys.size();++i){const int id=static_cast<int>(i%17);trie.add(static_cast<T>(keys[i]),id);model[keys[i]].push_back(id);}
    auto verify=[&] {
        std::size_t total=0;std::vector<U>ordered;for(const auto&[x,ids]:model){total+=ids.size();ordered.insert(ordered.end(),ids.size(),x);}
        check(trie.size()==total);check(trie.empty()==model.empty());
        for(U key:keys){
            const auto it=model.find(key);const auto count=it==model.end()?0:it->second.size();check(trie.count(static_cast<T>(key))==count);check(trie.contains(static_cast<T>(key))==(count!=0));
            auto ids=trie.find(static_cast<T>(key)).second;std::sort(ids.begin(),ids.end());std::vector<int>expected=it==model.end()?std::vector<int>{}:it->second;std::sort(expected.begin(),expected.end());check(ids==expected);
            const std::uint64_t bounds[]={0,1,static_cast<std::uint64_t>(mask/2),static_cast<std::uint64_t>(mask),std::numeric_limits<std::uint64_t>::max()};
            for(auto bound:bounds){std::size_t less=0,xless=0;U xlo=mask,xhi=0;for(const auto&[x,stored]:model){less+=stored.size()*(static_cast<std::uint64_t>(x)<bound);xless+=stored.size()*(static_cast<std::uint64_t>(x^key)<bound);xlo=std::min(xlo,static_cast<U>(x^key));xhi=std::max(xhi,static_cast<U>(x^key));}check(trie.count_less(bound)==less);check(trie.count_xor_less(static_cast<T>(key),bound)==xless);if(total){check(trie.min_xor(static_cast<T>(key)).first==static_cast<T>(xlo));check(trie.max_xor(static_cast<T>(key)).first==static_cast<T>(xhi));}}
            check(trie.count_less(std::int64_t{-1})==0);check(trie.count_xor_less(static_cast<T>(key),std::int64_t{-1})==0);
        }
        if(total){check(trie.min_element().first==static_cast<T>(ordered.front()));check(trie.max_element().first==static_cast<T>(ordered.back()));for(std::size_t i=0;i<ordered.size();++i)check(trie.get_kth(i+1).first==static_cast<T>(ordered[i]));}
    };
    verify();
    for(U x:{mask,static_cast<U>(mask/3),U{0},mask}){trie.operate_xor(static_cast<T>(x));decltype(model) next;for(auto&[key,ids]:model)next.emplace(key^x,std::move(ids));model=std::move(next);verify();}
    while(!model.empty()){auto item=model.begin();const auto key=item->first;const int id=item->second.back();check(trie.del(static_cast<T>(key),id));item->second.pop_back();if(item->second.empty())model.erase(item);}
    check(trie.empty());check(!trie.del(static_cast<T>(mask)));check(trie.insert_unique(static_cast<T>(mask),9));check(!trie.insert_unique(static_cast<T>(mask),10));check(trie.find(static_cast<T>(mask)).second==std::vector<int>{9});check(trie.del(static_cast<T>(mask),9));
}
}
int main(){
    test<std::uint8_t,0>(1);test<std::uint8_t,7>(2);test<std::int8_t,6>(3);test<std::uint16_t,11>(4);test<std::uint16_t,15>(5);test<std::uint32_t,30>(6);test<std::uint32_t,31>(7);test<std::int64_t,62>(8);test<std::uint64_t,63>(9);test<char8_t,7>(10);test<char16_t,15>(11);test<char32_t,31>(12);
    std::printf("fast BinaryTrie boundary checks=%ju\n",static_cast<std::uintmax_t>(checks));
}
