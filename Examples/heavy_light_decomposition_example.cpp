// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Heavy_Light_Decomposition.cpp"
#include "../DataStructure/Fenwick_Tree.cpp"
#include "../DataStructure/Segment_Tree.cpp"
#include "../DataStructure/Lazy_Segment_Tree.cpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
namespace {
std::string concat(std::string a,const std::string& b){a+=b;return a;}
std::string empty_word(){return {};}
struct Total{std::int64_t sum;int length;};
Total combine(Total a,Total b){return {a.sum+b.sum,a.length+b.length};}
Total empty_total(){return {0,0};}
Total add_total(std::int64_t f,Total x){return {x.sum+f*x.length,x.length};}
std::int64_t compose_add(std::int64_t newer,std::int64_t older){return newer+older;}
std::int64_t zero_add(){return 0;}
}
int main(){
 Graph tree{{1,2},{0,3,4},{0},{1},{1}};
 // 分解結果だけを使う場合。区間端は頂点番号ではなくDFS添字。
 HeavyLightDecomposition layout(tree);
 auto [l,r]=layout.subtree_range(1);
 assert(r-l==3&&layout.subtree_size(1)==3);
 auto parts=layout.path(3,2); // Intervalの配列を所有して取り出せる。
 std::vector<int> route;
 for(auto [left,right,reversed]:parts)
  for(int i=0;i<right-left;++i)
   route.push_back(layout.vertex_at(reversed?right-1-i:left+i));
 assert(route==std::vector<int>({3,1,0,2}));
 assert(layout.order()[layout.index(4)]==4);
 // 初期値は頂点番号順。内部の並べ替えを利用側で行う必要はない。
 std::vector<std::int64_t> value{1,2,3,4,5};
 auto hld=HeavyLightDecomposition(tree).with_values<fenwick_tree<std::int64_t>>(value);
 assert(hld.lca(3,4)==1&&hld.jump(3,2,2)==0&&hld.kth_ancestor(4,2)==0);
 assert(hld.path_prod(3,2)==10);
 hld.add(3,10);
 assert(value[3]==4); // 元の配列は独立しており、変更されない。
 auto current=hld.values();
 assert(hld.get(3)==14&&hld.path_prod(3,2)==20&&hld.subtree_prod(1)==21);
 assert(current==std::vector<std::int64_t>({1,2,3,14,5}));
 // Ordered=trueなら、経路の向きを変えた文字列も内部で正しく集計する。
 using Words=segtree<std::string,concat,empty_word>;
 auto words=HeavyLightDecomposition(tree).with_values<Words,true>(std::vector<std::string>{"a","b","c","d","e"});
 assert(words.path_prod(3,2)=="dbac");std::cout<<words.path_prod(3,2)<<'\n';
 words.set(3,"D");assert(words.path_prod(3,2)=="Dbac"&&words.path_prod(2,3)=="cabD");
 // Lazyの更新内容はここでは加算量。Sに和と要素数を持たせる。
 using Ranges=lazy_segtree<Total,combine,empty_total,std::int64_t,add_total,compose_add,zero_add>;
 std::vector<Total> initial;for(auto x:current)initial.push_back({x,1});
 auto ranges=HeavyLightDecomposition(tree).with_values<Ranges>(initial);
 ranges.path_apply(3,2,std::int64_t{2});assert(ranges.path_prod(3,2).sum==28);
 ranges.subtree_apply(1,std::int64_t{-1});assert(ranges.path_prod(3,2).sum==26);
 ranges.apply(4,std::int64_t{3});assert(ranges.get(4).sum==7);
 // 辺(0,1)=7、辺(1,2)=11。根0の値は単位元0にする。
 Graph chain{{1},{0,2},{1}};
 auto edges=HeavyLightDecomposition(chain).with_values<fenwick_tree<std::int64_t>>(std::vector<std::int64_t>{0,7,11});
 assert(edges.path_prod(0,2,true)==18&&edges.subtree_prod(1,true)==11);
 edges.set(2,13); // 子2に格納した親辺(1,2)を代入更新する。
 assert(edges.path_prod(0,2,true)==20&&edges.path_prod(2,2,true)==0&&edges.subtree_prod(2,true)==0);
 auto copy=hld;copy.set(0,100);assert(hld.get(0)==1); // コピーも独立。
 copy.build(chain,std::vector<std::int64_t>{2,4,6},0); // 根・形を変える際は値も渡す。
 assert(copy.path_prod(0,2)==12);
}
