// SPDX-License-Identifier: CC0-1.0
#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <new>
namespace { std::atomic<std::ptrdiff_t> budget{-1}; std::atomic<std::size_t> live{0}; }
void* operator new(std::size_t n) { if (budget==0) throw std::bad_alloc(); if (budget>0) --budget; if(void* p=std::malloc(n?n:1)){++live;return p;}throw std::bad_alloc(); }
void operator delete(void* p) noexcept {if(p){--live;std::free(p);}}
void operator delete(void* p,std::size_t) noexcept {::operator delete(p);}
void* operator new[](std::size_t n) {return ::operator new(n);}
void operator delete[](void* p) noexcept {::operator delete(p);}
void operator delete[](void* p,std::size_t) noexcept {::operator delete(p);}
#include "../../../Graph/Heavy_Light_Decomposition.cpp"
#include "../../../DataStructure/Fenwick_Tree.cpp"
#include "../../../DataStructure/Segment_Tree.cpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
namespace {
std::string join(std::string a,const std::string& b){a+=b;return a;}
std::string identity(){return {};}
Graph chain(int n){Graph g(n);for(int i=1;i<n;++i){g[i-1].push_back(i);g[i].push_back(i-1);}return g;}
}
int main(){
 const auto initial=live.load();std::size_t failures=0,successes=0;
 for(int b=0;b<256;++b){
  {
   const auto graph=chain(17);const std::vector<std::int64_t> values(17,7);HeavyLightDecomposition layout(graph,8);const auto before=live.load();budget=b;
   try{auto tree=layout.with_values<fenwick_tree<std::int64_t>>(values);budget=-1;assert(tree.path_prod(0,16)==119);++successes;}
   catch(const std::bad_alloc&){budget=-1;++failures;}
   assert(live==before&&layout.size()==17&&layout.root()==8);
  }
  {
   auto tree=HeavyLightDecomposition(chain(2)).with_values<fenwick_tree<std::int64_t>>(std::vector<std::int64_t>{2,3});const auto graph=chain(73);const std::vector<std::int64_t> values(73,11);const auto before=live.load();budget=b;
   try{tree.build(graph,values,36);budget=-1;assert(tree.size()==73&&tree.root()==36&&tree.path_prod(0,72)==803);++successes;}
   catch(const std::bad_alloc&){budget=-1;assert(live==before&&tree.size()==2&&tree.path_prod(0,1)==5);++failures;}
  }
  {
   using Range=segtree<std::string,join,identity>;const std::vector<std::string> values(17,std::string(100,'x'));auto tree=HeavyLightDecomposition(chain(2)).with_values<Range,true>(std::vector<std::string>{"a","b"});const auto graph=chain(17);const auto before=live.load();budget=b;
   try{tree.build(graph,values,8);budget=-1;assert(tree.path_prod(16,0)==std::string(1700,'x'));++successes;}
   catch(const std::bad_alloc&){budget=-1;assert(live==before&&tree.size()==2&&tree.path_prod(1,0)=="ba");++failures;}
  }
  {
   using Range=segtree<std::string,join,identity>;auto destination=HeavyLightDecomposition(chain(2)).with_values<Range,true>(std::vector<std::string>{"a","b"});auto source=HeavyLightDecomposition(chain(17)).with_values<Range,true>(std::vector<std::string>(17,std::string(100,'x')));const auto before=live.load();budget=b;
   try{destination=source;budget=-1;assert(destination.size()==17&&destination.path_prod(0,16)==std::string(1700,'x'));++successes;}
   catch(const std::bad_alloc&){budget=-1;assert(live==before&&destination.size()==2&&destination.path_prod(1,0)=="ba");++failures;}
   assert(source.size()==17&&source.get(3)==std::string(100,'x'));
  }
  assert(live==initial);
 }
 assert(failures>0&&successes>0);std::cout<<"HLD owned allocation failures="<<failures<<" successes="<<successes<<" live_delta="<<live.load()-initial<<'\n';
}
