// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>
#ifndef DUAL_SEGMENT_SOURCE
#define DUAL_SEGMENT_SOURCE "../../../DataStructure/Dual_Segment_Tree.cpp"
#endif
#include DUAL_SEGMENT_SOURCE
#include DUAL_SEGMENT_SOURCE
constexpr std::uint64_t mod=998244353;
struct F {std::uint32_t a,b;};
struct Map {std::uint32_t operator()(const F&f,std::uint32_t x)const{return (std::uint64_t{f.a}*x+f.b)%mod;}};
struct Comp {F operator()(const F&f,const F&g)const{return {static_cast<std::uint32_t>(std::uint64_t{f.a}*g.a%mod),static_cast<std::uint32_t>((std::uint64_t{f.a}*g.b+f.b)%mod)};}};
using Tree=DualSegmentTree<std::uint32_t,F,Map,Comp>;
struct NonDefault {int value;NonDefault()=delete;explicit NonDefault(int v):value(v){};};
struct NonDefaultAction {int value;NonDefaultAction()=delete;explicit NonDefaultAction(int v):value(v){};};
struct NonDefaultMap {NonDefault operator()(const NonDefaultAction& f,const NonDefault& x)const{return NonDefault(x.value+f.value);}};
struct NonDefaultComp {NonDefaultAction operator()(const NonDefaultAction& f,const NonDefaultAction& g)const{return NonDefaultAction(f.value+g.value);}};
struct MoveMap {std::unique_ptr<std::uint64_t> modulus;std::uint32_t operator()(const F&f,std::uint32_t x)const{return (std::uint64_t{f.a}*x+f.b)%*modulus;}};

struct MemberValue {int x;MemberValue transform()const{return {x*3};}};
struct MemberAction {
 int a,b;
 MemberValue mapping(const MemberValue& x)const{return {a*x.x+b};}
 MemberAction composition(const MemberAction& old)const{return {a*old.a,a*old.b+b};}
};
int main(){
 std::mt19937_64 rng(7319821);std::uint64_t operations=0;
 for(int trial=0;trial<180;++trial){
  std::size_t n=trial<100?trial:rng()%130;std::vector<std::uint32_t> a(n);for(auto&x:a)x=rng()%mod;
  Tree tree(a,Map{},Comp{});assert(tree.size()==n&&tree.empty()==(n==0));
  for(int q=0;q<2000;++q){
   std::size_t l=rng()%(n+1),r=rng()%(n+1);if(l>r)std::swap(l,r);F f{static_cast<std::uint32_t>(rng()%mod),static_cast<std::uint32_t>(rng()%mod)};
   int op=rng()%10;
   if(op<=3){tree.apply(l,r,f);for(std::size_t i=l;i<r;++i)a[i]=Map{}(f,a[i]);}
   else if(n&&op==4){auto k=rng()%n;tree.apply(k,f);a[k]=Map{}(f,a[k]);}
   else if(n&&op==5){auto k=rng()%n;auto x=static_cast<std::uint32_t>(rng()%mod);tree.set(k,x);a[k]=x;}
   else if(n&&op==6){auto k=rng()%n;tree.transform(k,[&](std::uint32_t x){return Map{}(f,x);});a[k]=Map{}(f,a[k]);}
   else if(n&&op==7){auto k=rng()%n;assert(tree[k]==a[k]);}
   else if(op==8){auto copy=tree;assert(copy.values()==a);if(n){copy.set(0,123);assert(tree.get(0)==a[0]);}auto moved=std::move(copy);assert(copy.empty()&&copy.usable());copy.reset(a);assert(copy.values()==a);moved=std::move(moved);}
   else assert(tree.values()==a);
   ++operations;
  }
  assert(tree.values()==a);auto copy=tree;tree.clear();assert(tree.empty());swap(tree,copy);assert(tree.values()==a);tree.reset(std::span<const std::uint32_t>(a));assert(tree.values()==a);
 }
 Tree empty(std::vector<std::uint32_t>{},Map{},Comp{});empty.apply(0,0,F{2,3});assert(empty.values().empty());
#ifndef SKIP_RUNTIME_CHECKS
 bool caught=false;try{empty.apply(1,0,F{2,3});}catch(const std::out_of_range&){caught=true;}assert(caught&&empty.usable());
 caught=false;try{empty.get(0);}catch(const std::out_of_range&){caught=true;}assert(caught&&empty.usable());
#endif
 std::vector<NonDefault> nd;nd.emplace_back(4);nd.emplace_back(8);DualSegmentTree<NonDefault,NonDefaultAction,NonDefaultMap,NonDefaultComp> nt(nd,{},{});nt.apply(0,2,NonDefaultAction(3));assert(nt.get(0).value==7&&nt.get(1).value==11);
 DualSegmentTree<std::uint32_t,F,MoveMap,Comp> mt(std::vector<std::uint32_t>{4,5},MoveMap{std::make_unique<std::uint64_t>(mod)},Comp{});mt.apply(0,2,F{2,3});auto mc=mt;assert(mc.get(1)==13);auto mm=std::move(mt);mt.reset(3,4);assert(mt.get(2)==4&&mm.get(0)==11);

 DualSegmentTree<MemberValue,MemberAction,decltype(&MemberAction::mapping),decltype(&MemberAction::composition)> member(std::vector<MemberValue>{{1},{2},{3}},&MemberAction::mapping,&MemberAction::composition);
 member.apply(0,3,MemberAction{2,1});member.apply(1,3,MemberAction{3,2});assert(member.get(0).x==3&&member.get(1).x==17&&member.get(2).x==23);member.transform(1,&MemberValue::transform);assert(member.get(1).x==51);
 // word境界・大きな非2冪、全区間と1点を交互に作用。
 std::vector<std::uint32_t> large(200003,1);Tree big(large,{},{});for(int i=0;i<120;++i){F f{2,static_cast<std::uint32_t>(i)};big.apply(0,large.size(),f);for(auto&x:large)x=Map{}(f,x);auto k=static_cast<std::size_t>(rng()%large.size());big.set(k,i);large[k]=i;}assert(big.values()==large);
 std::cout<<"dual reference operations="<<operations<<" large=200003 OK\n";
}
