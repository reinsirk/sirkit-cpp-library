// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_PUF_SOURCE
#define SIRKIT_PUF_SOURCE "../../../DataStructure/persistent-union-find.cpp"
#endif
#include SIRKIT_PUF_SOURCE
#include <cstdint>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <numeric>
#include <random>
#include <vector>
#include <limits>
std::uint64_t checks=0;
int main(){std::mt19937_64 rng(1251994);for(int n:{1,16,17,257,4097,65537,1048577,std::numeric_limits<int>::max()}){
 std::vector<int>keys{0,n-1,n/2};while(keys.size()<std::size_t(std::min(n,128))){keys.push_back(int(rng()%std::uint64_t(n)));std::sort(keys.begin(),keys.end());keys.erase(std::unique(keys.begin(),keys.end()),keys.end());}std::sort(keys.begin(),keys.end());keys.erase(std::unique(keys.begin(),keys.end()),keys.end());int k=int(keys.size());PersistentUnionFind h(n);std::vector<PersistentUnionFind::Node*>versions{h.get_root()};std::vector<std::vector<int>>models(1,std::vector<int>(k));std::iota(models[0].begin(),models[0].end(),0);
 auto add=[&](int a,int b,std::size_t ver){auto model=models[ver];int from=model[b],to=model[a];bool joined=from!=to;if(joined)for(auto&v:model)if(v==from)v=to;assert(h.unite(keys[a],keys[b],versions[ver])==joined);++checks;versions.push_back(h.get_root());models.push_back(std::move(model));};
 for(int step=1;step<k;step*=2)for(int i=0;i+step<k;i+=2*step)add(i,i+step,versions.size()-1);
 for(int i=0;i<500;++i)add(int(rng()%k),int(rng()%k),std::size_t(rng()%versions.size()));
 for(std::size_t ver=0;ver<versions.size();++ver){auto*root=versions[ver];const auto&m=models[ver];for(int q=0;q<48;++q){int a=int(rng()%k),b=int(rng()%k),v=keys[a];int leader=h.find(v,root);auto it=std::lower_bound(keys.begin(),keys.end(),leader);assert(it!=keys.end()&&*it==leader);assert(m[std::size_t(it-keys.begin())]==m[a]);assert(h.leader(v,root)==leader&&h.root_of(v,root)==leader);assert(h.find_with_size(v,root).first==leader);assert(h.size(v,root)==std::count(m.begin(),m.end(),m[a]));assert(h.same(v,keys[b],root)==(m[a]==m[b]));checks+=6;
  int parent=h.arr.get(root,v),steps=0;
  while(parent>=0){int next=h.arr.get(root,parent);assert(next!=-1);assert(++steps<=k);parent=next;++checks;}
 }
  if(n>k){int v=int(rng()%std::uint64_t(n));if(!std::binary_search(keys.begin(),keys.end(),v)){assert(h.find(v,root)==v&&h.size(v,root)==1);++checks;}}
 }
 if(n<=1048577){auto ver=std::size_t(k>1?k-1:0);auto groups=h.groups(versions[ver]);auto active=models[ver];std::sort(active.begin(),active.end());active.erase(std::unique(active.begin(),active.end()),active.end());assert(groups.size()==std::size_t(n-k)+active.size());++checks;std::vector<int>seen(n);for(const auto&g:groups){assert(std::is_sorted(g.begin(),g.end()));assert(!g.empty());assert(g.size()==std::size_t(h.size(g.front(),versions[ver])));++checks;for(int v:g){assert(0<=v&&v<n);assert(h.find(v,versions[ver])==h.find(g.front(),versions[ver]));assert(++seen[v]==1);++checks;}}assert(std::all_of(seen.begin(),seen.end(),[](int x){return x==1;}));}
#ifndef SKIP_RUNTIME_CHECKS
 bool threw=false;try{h.find(-1);}catch(const std::out_of_range&){threw=true;}assert(threw);++checks;PersistentUnionFind other(n);threw=false;try{h.find(0,other.get_root());}catch(const std::invalid_argument&){threw=true;}assert(threw);++checks;
#endif
 }std::printf("%ju large groups/owner/parent-path invariant checks passed\n",static_cast<std::uintmax_t>(checks));}
