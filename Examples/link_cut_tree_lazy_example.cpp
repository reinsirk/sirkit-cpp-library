// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Link_Cut_Tree.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
using I=std::int64_t;
struct Affine {I a,b;};
struct Mapping {I operator()(const Affine&f,I sum,std::size_t n)const{return f.a*sum+f.b*static_cast<I>(n);}};
struct Composition {Affine operator()(const Affine&newer,const Affine&older)const{return {newer.a*older.a,newer.a*older.b+newer.b};}};
int main(){
    LinkCutTree<I,std::plus<I>,Affine,Mapping,Composition> tree(std::vector<I>{1,2,3,4,5},0);
    tree.link(0,2);tree.link(2,1);tree.link(2,3);tree.link(3,4);
    tree.apply(0,4,Affine{2,1});
    std::cout<<*tree.path_prod(0,4)<<'\n'; // 30
    tree.apply(1,Affine{3,-1});
    std::cout<<*tree.path_prod(1,4)<<'\n'; // 32
    tree.apply(4,0,Affine{0,6});
    std::cout<<*tree.path_prod(1,4)<<'\n'; // 23
    tree.cut(2,3);if(tree.apply(1,4,Affine{1,100}))return 1;
    assert(tree.get(1)==5&&tree.get(4)==6);
}
