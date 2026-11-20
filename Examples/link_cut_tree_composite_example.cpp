// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Link_Cut_Tree.cpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace sirkit;
struct Affine {std::uint32_t a,b;};
struct Compose {
    static constexpr std::uint64_t mod=998244353;
    Affine operator()(const Affine&first,const Affine&second)const{return {static_cast<std::uint32_t>(std::uint64_t{first.a}*second.a%mod),static_cast<std::uint32_t>((std::uint64_t{first.b}*second.a+second.b)%mod)};}
};
int main(){
    LinkCutTree<Affine,Compose> tree(std::vector<Affine>{{2,1},{3,4},{5,6}},Affine{1,0});
    tree.link(0,1);tree.link(1,2);
    const auto forward=*tree.path_prod(0,2),backward=*tree.path_prod(2,0);
    std::cout<<(std::uint64_t{forward.a}*7+forward.b)%Compose::mod<<'\n'; // 251
    std::cout<<(std::uint64_t{backward.a}*7+backward.b)%Compose::mod<<'\n'; // 255
    tree.set(1,Affine{0,9});
    assert(tree.path_prod(0,2)->b==51&&tree.path_prod(2,0)->b==19);
}
