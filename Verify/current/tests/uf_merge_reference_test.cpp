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
static void require(bool value){++checks;if(!value)std::abort();}
// 各頂点の代表を直接書換えるモデル。親木や経路圧縮を使わない。
struct Model {
    std::vector<int> labels, counts;
    int components;
    explicit Model(int n):labels(n),counts(n,1),components(n){std::iota(labels.begin(),labels.end(),0);}
    bool merge(int x,int y){x=labels[x];y=labels[y];if(x==y)return false;if(counts[x]<counts[y])std::swap(x,y);for(auto&v:labels)if(v==y)v=x;counts[x]+=counts[y];counts[y]=0;--components;return true;}
    auto groups()const{std::vector<std::vector<int>> out(labels.size());for(int i=0;i<static_cast<int>(labels.size());++i)out[labels[i]].push_back(i);std::erase_if(out,[](const auto&v){return v.empty();});return out;}
};
static void inspect(UnionFind&uf,const Model&m){
    require(uf.size()==static_cast<int>(m.labels.size()));require(uf.component_count()==m.components);
    for(int v=0;v<uf.size();++v){require(uf.root(v)==m.labels[v]);require(uf.leader(v)==m.labels[v]);require(uf.size(v)==m.counts[m.labels[v]]);}
    require(uf.groups()==m.groups());
}
static void exercise(int n,int block,std::mt19937&random){
    UnionFind uf(n);Model model(n);std::vector<int> ids(n);std::iota(ids.begin(),ids.end(),0);std::shuffle(ids.begin(),ids.end(),random);
    auto merge=[&](int x,int y,bool alias=false){const bool expected=model.merge(x,y);if(alias)uf.unite(x,y);else require(uf.merge(x,y)==expected);require(uf.component_count()==model.components);require(uf.root(x)==model.labels[x]);require(uf.size(y)==model.counts[model.labels[y]]);};
    for(int start=0;start<n;start+=block){const int end=std::min(n,start+block);for(int stride=1;stride<end-start;stride*=2)for(int i=start;i+stride<end;i+=stride*2)merge(ids[i],ids[i+stride],(i+stride)%3==0);}
    // 圧縮前の深い端点で、別成分/同一成分と双方のサイズ順を通す。
    for(int end=block*2;end<=n;end+=block*2){merge(ids[end-1],ids[end-block-1]);merge(ids[end-block-1],ids[end-1]);}
    inspect(uf,model);
    if(n){for(int step=0;step<4000;++step){int x=static_cast<int>(random()%n),y=step%7?static_cast<int>(random()%n):x;merge(x,y,step%11==0);if(step%127==0)inspect(uf,model);}inspect(uf,model);}
    auto copy=uf;auto copied=model;inspect(copy,copied);uf.reset();model=Model(n);inspect(uf,model);inspect(copy,copied);
    for(int count:{0,1,17,257}){uf.reset(count);model=Model(count);if(count>1)merge(count-1,0);inspect(uf,model);}
}
int main(){std::mt19937 random(20261030);for(int n:{0,1,2,3,17,257,4097})for(int block:{2,16,128,8192})exercise(n,block,random);std::printf("PASS exact roots/sizes/groups checks=%ju\n",static_cast<std::uintmax_t>(checks));}
