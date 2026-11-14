// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <iostream>
#include <vector>
#include "../DataStructure/Dual_Segment_Tree.cpp"

using namespace sirkit;
struct Affine {std::int64_t a,b;};
struct Mapping {std::int64_t operator()(const Affine&f,std::int64_t x)const{return f.a*x+f.b;}};
struct Composition {Affine operator()(const Affine&f,const Affine&g)const{return {f.a*g.a,f.a*g.b+f.b};}};
int main(){
 DualSegmentTree<std::int64_t,Affine,Mapping,Composition> tree(std::vector<std::int64_t>{1,2,3,4},{},{});
 tree.apply(0,3,Affine{2,1});tree.apply(1,4,Affine{3,2});
 std::cout<<tree.get(1)<<'\n'; // 17
 tree.set(1,10);tree.apply(1,Affine{2,-1});tree.transform(3,[](std::int64_t x){return x+1;});
 for(auto x:tree.values())std::cout<<x<<' ';std::cout<<'\n'; // 3 19 23 15
 auto copy=tree;copy.apply(0,4,Affine{0,8});
 std::cout<<tree.get(0)<<' '<<copy.get(0)<<'\n'; // 3 8
 tree.clear();tree.reset(2,7);std::cout<<tree.size()<<' '<<tree[1]<<'\n'; // 2 7
}
