// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef LINK_CUT_SOURCE
#define LINK_CUT_SOURCE "../../../DataStructure/Link_Cut_Tree.cpp"
#endif
#include LINK_CUT_SOURCE
#include <cassert>
#include <iostream>
#include <numeric>
struct HashValue { std::uint64_t hash; std::size_t length; friend bool operator==(const HashValue&,const HashValue&)=default; };
struct Tables {
    static constexpr std::uint64_t base=0x1f123bb5;
    std::vector<std::uint64_t> power,geometric;
    explicit Tables(std::size_t n):power(n+1,1),geometric(n+1,0){for(std::size_t i=1;i<=n;++i){power[i]=power[i-1]*base;geometric[i]=geometric[i-1]*base+1;}}
};
struct HashOp {
    std::shared_ptr<const Tables> table;
    HashValue operator()(const HashValue&a,const HashValue&b)const noexcept{return {a.hash*table->power[b.length]+b.hash,a.length+b.length};}
};
struct HashAction { std::uint64_t a,b; };
struct HashMapping {
    std::shared_ptr<const Tables> table;
    HashValue operator()(const HashAction&f,const HashValue&v,std::size_t count)const noexcept{assert(count==v.length);return {f.a*v.hash+f.b*table->geometric[count],count};}
};
struct HashComposition { HashAction operator()(const HashAction&f,const HashAction&g)const noexcept{return {f.a*g.a,f.a*g.b+f.b};} };
using Tree=LinkCutTree<HashValue,HashOp,HashAction,HashMapping,HashComposition>;
int main(){
    constexpr int n=300000;assert(std::gcd(n,100003)==1);
    auto tables=std::make_shared<Tables>(n);
    std::vector<int> order(n);std::vector<HashValue> initial(n);std::vector<std::uint64_t> values(n);
    for(int i=0;i<n;++i){order[i]=static_cast<int>((static_cast<std::uint64_t>(i)*100003+17)%n);values[i]=static_cast<std::uint64_t>(i)*0x9e3779b97f4a7c15+1;initial[i]={values[i],1};}
    Tree tree(initial,HashValue{0,0},HashOp{tables},HashMapping{tables});
    for(int i=1;i<n;++i)assert(tree.link(order[i-1],order[i]));
    auto expected=[&](bool reverse){std::uint64_t h=0;for(int i=0;i<n;++i)h=h*Tables::base+values[order[reverse?n-1-i:i]];return HashValue{h,n};};
    assert(tree.path_prod(order.front(),order.back())==expected(false));
    assert(tree.path_prod(order.back(),order.front())==expected(true));
    assert(tree.path_size(order.front(),order.back())==n&&tree.path_length(order.front(),order.back())==n-1);
    for(std::size_t k:{std::size_t{0},std::size_t{1},std::size_t{63},std::size_t{64},std::size_t{n/2},std::size_t{n-2},std::size_t{n-1}}){assert(tree.kth_on_path(order.front(),order.back(),k)==order[k]);assert(tree.kth_on_path(order.back(),order.front(),k)==order[n-1-k]);}
    assert(!tree.kth_on_path(order.front(),order.back(),n));
    constexpr int left=12345,right=n-1000;
    assert(tree.apply(order[left],order[right],HashAction{3,7}));
    for(int i=left;i<=right;++i)values[order[i]]=values[order[i]]*3+7;
    assert(tree.path_prod(order.front(),order.back())==expected(false));
    tree.transform(order[n/2],[](HashValue v){v.hash+=13;return v;});values[order[n/2]]+=13;
    assert(tree.get(order[n/2])==(HashValue{values[order[n/2]],1}));
    assert(tree.path_prod(order.back(),order.front())==expected(true));
    assert(tree.cut(order[n/2-1],order[n/2]));
    assert(!tree.connected(order.front(),order.back()));
    assert(!tree.path_prod(order.front(),order.back())&&tree.path_size(order.front(),order.back())==0&&!tree.path_length(order.front(),order.back()));
    assert(!tree.kth_on_path(order.front(),order.back(),0));
    assert(!tree.lca(order.front(),order[n/3],order[2*n/3]));
    assert(tree.link(order[n/2],order[n/2-1]));
    assert(tree.path_prod(order.front(),order.back())==expected(false));
    assert(tree.lca(order.front(),order[n/3],order[2*n/3])==order[n/3]);
    assert(tree.lca(order.back(),order[n/3],order[2*n/3])==order[2*n/3]);
    tree.make_root(order[11111]);assert(tree.find_root(order.front())==order[11111]&&tree.find_root(order.back())==order[11111]);
    auto copy=tree;copy.apply(order.front(),order.back(),HashAction{0,5});assert(copy.path_prod(order.front(),order.back())==(HashValue{5*tables->geometric[n],n}));
    assert(tree.path_prod(order.front(),order.back())==expected(false));
    auto moved=std::move(copy);assert(copy.empty()&&copy.usable());copy.reset(std::vector<HashValue>{{42,1}});assert(copy.get(0)==(HashValue{42,1}));
    moved.clear();assert(moved.empty());
    std::cout<<"link cut 300000 arbitrary-label chain, noncommutative hash, lazy affine, cut/link/LCA/ownership OK\n";
}
