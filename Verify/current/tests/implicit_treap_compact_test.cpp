// SPDX-License-Identifier: CC0-1.0
#include <cstdlib>
#ifndef HEADER
#define HEADER "../../../DataStructure/implicit_treap.cpp"
#endif
#include HEADER
#include <cassert>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
struct Affine {
 static constexpr std::int64_t mod=998244353;
 struct S{std::int64_t sum,count;friend bool operator==(S,S)=default;};struct F{std::int64_t a,b;};
 static S op(S x,S y){return{(x.sum+y.sum)%mod,x.count+y.count};}static S e(){return{0,0};}
 static S mapping(F f,S x){return{(f.a*x.sum+f.b*x.count)%mod,x.count};}
 static F composition(F f,F g){return{f.a*g.a%mod,(f.a*g.b+f.b)%mod};}static F id(){return{1,0};}
};
struct Conjugation {
 struct S{std::uint32_t a,b;friend bool operator==(S,S)=default;};struct F{std::int64_t a,b,tag;};
 static S op(S x,S y){return{std::uint32_t(x.a*y.a%17),std::uint32_t((x.b*y.a+y.b)%17)};}static S e(){return{1,0};}
 static S mapping(F f,S x){auto b=(f.a*x.b+(1-std::int64_t(x.a))*f.b)%17;return{x.a,std::uint32_t((b+17)%17)};}
 static F composition(F f,F g){return{f.a*g.a%17,(f.a*g.b+f.b)%17,0};}static F id(){return{1,0,0};}
};
template<class A,bool comm>std::uint64_t run(){using S=typename A::S;using F=typename A::F;using T=lazy_implicit_treap<S,A::op,A::e,F,A::mapping,A::composition,A::id,comm>;
#ifdef REVIEW_EXPOSED
 static_assert(T::compact_nodes_);static_assert(T::deferred_actions_==comm);static_assert(sizeof(typename T::Node)==64);
#endif
 std::mt19937_64 gen(comm?18319:21892);std::vector<S>v;T t;std::uint64_t checks=0;
 auto value=[&](){if constexpr(comm)return S{std::int64_t(gen()%998244353),1};else return S{std::uint32_t(gen()%17),std::uint32_t(gen()%17)};};
 auto action=[&](){if constexpr(comm)return F{std::int64_t(gen()%998244353),std::int64_t(gen()%998244353)};else return F{std::int64_t(gen()%16+1),std::int64_t(gen()%17),0};};
 auto fold=[&](int l,int r){S s=A::e();for(int i=l;i<r;++i)s=A::op(s,v[i]);return s;};
 for(int i=0;i<64;++i){auto s=value();v.push_back(s);t.push_back(s);}
 for(int q=0;q<30000;++q){int n=int(v.size()),l=int(gen()%(n+1)),r=int(gen()%(n+1));if(l>r)std::swap(l,r);int mode=int(gen()%12);
 if(mode==0&&n<110){auto s=value();t.insert_at(l,s);v.insert(v.begin()+l,s);}
 else if(mode==1&&n>8){r=std::min(l+1,n);auto got=t.erase_prod(l,r);assert(got==fold(l,r));v.erase(v.begin()+l,v.begin()+r);++checks;}
 else if(mode==2){std::reverse(v.begin()+l,v.begin()+r);t.reverse(l,r);}
 else if(mode==3||mode==4){if(mode==4){l=0;r=n;}auto f=action();t.apply(l,r,f);for(int i=l;i<r;++i)v[i]=A::mapping(f,v[i]);}
 else if(mode==5){int m=l+int(gen()%(r-l+1));t.rotate(l,m,r);std::rotate(v.begin()+l,v.begin()+m,v.begin()+r);}
 else if(mode==6){int pos=int(gen()%(n+1));t.move_range(l,r,pos);if(pos<l)std::rotate(v.begin()+pos,v.begin()+l,v.begin()+r);else if(pos>r)std::rotate(v.begin()+l,v.begin()+r,v.begin()+pos);}
 else if(mode==7&&n){int k=int(gen()%n);auto s=value();t.set(k,s);v[k]=s;}
 else if(mode==8){T c(t);assert(c.prod(l,r)==fold(l,r));T d;d=c;assert(d.prod(l,r)==fold(l,r));T moved(std::move(d));assert(d.empty());assert(moved.prod(l,r)==fold(l,r));checks+=3;}
 else if(mode==9&&n){int k=int(gen()%n);assert(t.get(k)==v[k]);++checks;}
 else if(mode==10){auto f=action();for(int repeat=0;repeat<3;++repeat){t.apply(0,n,f);for(auto&x:v)x=A::mapping(f,x);}if(n>2){t.reverse(1,n-1);std::reverse(v.begin()+1,v.end()-1);}}
 assert(t.all_prod()==fold(0,int(v.size())));++checks;n=int(v.size());l=int(gen()%(n+1));r=int(gen()%(n+1));if(l>r)std::swap(l,r);assert(t.prod(l,r)==fold(l,r));assert(t.prod(0,r)==fold(0,r));assert(t.prod(l,n)==fold(l,n));checks+=3;
 if(q%101==0){assert(t.to_vector()==v);++checks;}
 }
 return checks;
}
int main(){auto a=run<Affine,true>(),b=run<Conjugation,false>();std::cout<<a<<" compact deferred commutative and "<<b<<" compact noncommutative oracle checks passed\n";}
