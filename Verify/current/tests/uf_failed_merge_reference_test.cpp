// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_UF_SOURCE
#define SIRKIT_UF_SOURCE "../../../DataStructure/union_find.cpp"
#endif
#include SIRKIT_UF_SOURCE
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>
static std::uint64_t checks=0;
static void require(bool ok){++checks;if(!ok)std::abort();}
int main(){std::mt19937 random(103030);for(int n:{16,256,8192})for(int order:{0,1,2})for(int kind=0;kind<5;++kind){UnionFind uf(n);std::vector<int>ids(n);std::iota(ids.begin(),ids.end(),0);if(order==1)std::reverse(ids.begin(),ids.end());if(order==2)std::shuffle(ids.begin(),ids.end(),random);for(int stride=1;stride<n;stride*=2)for(int i=0;i+stride<n;i+=2*stride)require(uf.merge(ids[i],ids[i+stride]));
 // root/size/groupsを先に呼ばず、深いb側で既知根へ到達する場合を通す。
 const int root=ids[0],leaf=ids[n-1];int a=root,b=leaf;if(kind==1)std::swap(a,b);if(kind==2)a=leaf;if(kind==3){a=leaf;b=ids[n-3];}if(kind==4)uf.unite(a,b);else require(!uf.merge(a,b));require(uf.component_count()==1);for(int i=0;i<n;++i){require(uf.root(i)==root);require(uf.size(i)==n);}require(!uf.merge(root,root));require(uf.component_count()==1);}
 std::printf("PASS cold failed merges, known root/delayed root/root-self checks=%ju\n",static_cast<std::uintmax_t>(checks));}
