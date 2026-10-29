// SPDX-License-Identifier: CC0-1.0
#include <cstdlib>
#include <new>
#include <limits>
#include <cstdint>
#include <cassert>
#include <vector>
#include <utility>
#include <iostream>
#include <stdexcept>
static bool fail_identity=false;
static int budget=-1;static std::size_t ordinary=0,aligned=0;
static void hit(){if(budget==0)throw std::bad_alloc();if(budget>0)--budget;}
void*operator new(std::size_t n){hit();if(auto*p=std::malloc(n?n:1)){++ordinary;return p;}throw std::bad_alloc();}
void operator delete(void*p)noexcept{std::free(p);}void operator delete(void*p,std::size_t)noexcept{std::free(p);}
void*operator new(std::size_t n,std::align_val_t a){hit();const auto alignment=std::size_t(a),size=n?n:1;if(size>std::numeric_limits<std::size_t>::max()-(alignment-1))throw std::bad_alloc();void*p=std::aligned_alloc(alignment,((size+alignment-1)/alignment)*alignment);if(!p)throw std::bad_alloc();++aligned;return p;}
void operator delete(void*p,std::align_val_t)noexcept{std::free(p);}void operator delete(void*p,std::size_t,std::align_val_t)noexcept{std::free(p);}
#ifndef HEADER
#define HEADER "../../../DataStructure/implicit_treap.cpp"
#endif
#include HEADER
struct A {
struct S {std::int64_t sum,count;};struct F {std::int64_t a,b;};
static S op(S x,S y){return{(x.sum+y.sum)%998244353,x.count+y.count};}static S e(){return{0,0};}
static S mapping(F f,S x){return{(f.a*x.sum+f.b*x.count)%998244353,x.count};}
static F composition(F f,F g){return{f.a*g.a%998244353,(f.a*g.b+f.b)%998244353};}static F id(){if(fail_identity)throw std::runtime_error("identity");return{1,0};}
};
using T=lazy_implicit_treap<A::S,A::op,A::e,A::F,A::mapping,A::composition,A::id,true>;
using V=std::vector<std::pair<int,std::int64_t>>;
V snap(T&t){V v;for(int i=0;i<t.size();++i)v.emplace_back(i,t.get(i).sum);return v;}
void check(T&t,V const&v){assert(snap(t)==v);}
T tree(int n){T t;for(int i=0;i<n;++i)t.insert_at(i,{i+1,1});t.reverse(1,n-1);t.apply(0,n,{1,9});return t;}
int main(){std::size_t checks=0,fails=0;
for(int mode=0;mode<5;++mode)for(int b=0;b<12;++b){T src=tree(37),t=tree(5);if(mode==1)t.reserve(128);if(mode==3){for(int k=0;k<4;++k)t.erase_at(0);}auto before=snap(t),s=snap(src);bool failed=false;
budget=b;try{if(mode<=1)t=src;else if(mode==2)t.reserve(193);else if(mode==3)t.insert_at(0,{44,1});else {T copied(src);t.swap(copied);}}catch(std::bad_alloc const&){failed=true;}budget=-1;
if(failed){++fails;check(t,before);}else if(mode<=1||mode==4)check(t,s);else if(mode==2)check(t,before);else{auto e=before;e.insert(e.begin(),{0,44});for(int k=0;k<int(e.size());++k)e[k].first=k;check(t,e);}check(src,s);t.insert_at(0,{100,1});assert(t.front().sum==100);++checks;}
// Empty/moved-from/swap and retained free slots must keep parallel arrays coherent.
for(int b=0;b<10;++b){T t;budget=b;bool failed=false;try{t.insert_at(0,{5,1});}catch(std::bad_alloc const&){failed=true;}budget=-1;if(failed){assert(t.empty());++fails;t.insert_at(0,{5,1});}assert(t.front().sum==5);T u(std::move(t));assert(t.empty());t.insert_at(0,{8,1});u.swap(t);assert(u.front().sum==8&&t.front().sum==5);u.clear();u.insert_at(0,{9,1});++checks;}
// Force node capacity to outgrow priority capacity, then fail the priority copy.
{T source=tree(37),target=tree(5);auto before=snap(target),wanted=snap(source);
 budget=1;bool failed=false;try{target.reserve(193);}catch(std::bad_alloc const&){failed=true;}budget=-1;assert(failed);check(target,before);++fails;
 budget=0;failed=false;try{target=source;}catch(std::bad_alloc const&){failed=true;}budget=-1;assert(failed);check(target,before);++fails;
 target=source;check(target,wanted);checks+=3;}
// F is trivially copyable but id() may throw during fresh node construction.
{T t=tree(13);auto before=snap(t);fail_identity=true;bool failed=false;try{t.insert_at(4,{73,1});}catch(std::runtime_error const&){failed=true;}fail_identity=false;assert(failed);check(t,before);++fails;t.insert_at(4,{73,1});auto expected=before;expected.insert(expected.begin()+4,{4,73});for(int k=0;k<int(expected.size());++k)expected[k].first=k;check(t,expected);checks+=2;}
std::cout<<checks<<" cases "<<fails<<" failures ordinary="<<ordinary<<" aligned="<<aligned<<"\n";assert(aligned>0&&fails>0);
}
