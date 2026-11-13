// SPDX-License-Identifier: CC0-1.0
#ifndef FAST_NODE_SOURCE
#define FAST_NODE_SOURCE "../../../Fast/GCC/DataStructure/Node.cpp"
#endif
#include FAST_NODE_SOURCE
#include FAST_NODE_SOURCE
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <utility>
#include <vector>
namespace { std::uint64_t checks=0; void check(bool ok){++checks;if(!ok)std::abort();} }
int main(){
    Node* tail=make_chain(3);std::vector<std::pair<Node*,std::size_t>>chains;std::vector<Node*>all;
    for(int repeat=0;repeat<4;++repeat)for(std::size_t count:{0,1,2,4094,4095,4096,4097,8192,16,4096,1}){
        Node* h=make_chain(count,tail);check(chain_size(h)==count+3);check(chain_nth(h,count)==tail);chains.emplace_back(h,count);
        for(std::size_t i=0;i<count;++i){check(chain_nth(h,std::min<std::size_t>(i,3))==h+std::min<std::size_t>(i,3));all.push_back(h+i);}
        Node* single=make_node(tail);all.push_back(single);check(single->n==tail);
    }
    for(auto[first,count]:chains){for(std::size_t i=0;i<count;++i)check(first[i].n==(i+1<count?first+i+1:tail));Node* copy=clone_chain(first);check(chain_size(copy)==count+3);check(copy!=first);}
    std::sort(all.begin(),all.end(),std::less<Node*>{});check(std::adjacent_find(all.begin(),all.end())==all.end());
    Node a(tail);Node b=a;check(b.n==tail);b.n=nullptr;check(a.n==tail);Node c=std::move(a);check(c.n==tail);check(a.n==tail);
    std::printf("fast Node pool boundary checks=%ju unique_nodes=%zu\n",static_cast<std::uintmax_t>(checks),all.size());
}
