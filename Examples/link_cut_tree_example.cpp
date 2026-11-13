// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Link_Cut_Tree.cpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>
int main(){
    using I=std::int64_t;
    LinkCutTree<I> tree(std::vector<I>{2,4,1,7});
    tree.link(0,1);tree.link(1,2);tree.link(1,3);
    std::cout<<*tree.path_prod(2,3)<<'\n'; // 12
    tree.transform(1,[](I value){return value+5;});
    std::cout<<*tree.path_prod(0,2)<<'\n'; // 12
    assert(tree.path_size(0,3)==3&&tree.path_length(0,3)==2);
    assert(tree.kth_on_path(0,3,1)==1&&tree.lca(0,2,3)==1);
    tree.set(3,10);assert(tree.get(3)==10);
    tree.cut(1,3);assert(!tree.connected(0,3)&&!tree.path_prod(0,3));
    tree.link(3,2);
    std::cout<<*tree.path_prod(0,3)<<'\n'; // 22
    tree.make_root(3);assert(tree.find_root(0)==3);
    auto copy=tree;copy.set(0,100);assert(tree.get(0)==2);
    auto moved=std::move(copy);assert(copy.empty()&&moved.usable());
    copy.reset(std::vector<I>{8,9});assert(!copy.connected(0,1)&&copy.get(0)==8);
    tree.clear();assert(tree.empty()&&tree.usable());
}
