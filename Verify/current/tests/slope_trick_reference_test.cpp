// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>
#ifndef SLOPE_TRICK_SOURCE
#define SLOPE_TRICK_SOURCE "../../../DataStructure/Slope_Trick.cpp"
#endif
#include SLOPE_TRICK_SOURCE
#include SLOPE_TRICK_SOURCE
using I=std::int64_t;
constexpr int lo=-600,hi=600;
constexpr I inf=I{1}<<60;
void verify(const SlopeTrick& f,const std::vector<I>& a){
 const auto minimum=*std::min_element(a.begin(),a.end());assert(f.min_value()==minimum);
 for(int x=-80;x<=80;++x)assert(f.eval(x)==a[x-lo]);
 const auto range=f.argmin();int left=lo,right=hi;while(a[left-lo]!=minimum)++left;while(a[right-lo]!=minimum)--right;
 if(left<-400)assert(!range.lower);else assert(range.lower&&*range.lower==left);
 if(right>400)assert(!range.upper);else assert(range.upper&&*range.upper==right);
}
int main(){
 std::mt19937_64 rng(3995123);std::uint64_t operations=0;
 for(int trial=0;trial<100;++trial){
  SlopeTrick f;std::vector<I>a(hi-lo+1,0);
  for(int q=0;q<120;++q){int op=rng()%12;I p=static_cast<I>(rng()%61)-30;
   if(op<=2){f.add_abs(p);for(int x=lo;x<=hi;++x)a[x-lo]+=std::abs(static_cast<I>(x)-p);}
   else if(op==3){f.add_left_hinge(p);for(int x=lo;x<=hi;++x)a[x-lo]+=std::max<I>(p-x,0);}
   else if(op==4){f.add_right_hinge(p);for(int x=lo;x<=hi;++x)a[x-lo]+=std::max<I>(x-p,0);}
   else if(op==5){f.add_constant(p);for(auto&v:a)v+=p;}
   else if(op==6){I l=static_cast<I>(rng()%7)-3,r=static_cast<I>(rng()%7)-3;if(l>r)std::swap(l,r);f.shift(l,r);auto old=a;for(int x=lo;x<=hi;++x){a[x-lo]=inf;for(I y=x-r;y<=x-l;++y)if(lo<=y&&y<=hi)a[x-lo]=std::min(a[x-lo],old[y-lo]);}}
   else if(op==7){f.clear_right();for(int x=lo+1;x<=hi;++x)a[x-lo]=std::min(a[x-lo],a[x-lo-1]);}
   else if(op==8){f.clear_left();for(int x=hi-1;x>=lo;--x)a[x-lo]=std::min(a[x-lo],a[x-lo+1]);}
   else if(op==9){SlopeTrick g;I b=static_cast<I>(rng()%61)-30;g.add_abs(p);g.add_left_hinge(b);g.shift(2);f.merge(g);for(int x=lo;x<=hi;++x)a[x-lo]+=std::abs(static_cast<I>(x)-2-p)+std::max<I>(b-(x-2),0);assert(g.eval(p+2)==std::max<I>(b-p,0));}
   else if(op==10){auto copy=f;verify(copy,a);auto moved=std::move(copy);assert(copy.empty()&&copy.min_value()==0);copy.add_abs(3);assert(copy.eval(0)==3);moved=std::move(moved);auto assigned=SlopeTrick{};assigned=moved;verify(assigned,a);swap(assigned,copy);verify(copy,a);}
   else{f.reserve(f.size()+20,f.size()+20);I d=static_cast<I>(rng()%5)-2;f.shift(d);auto old=a;for(int x=lo;x<=hi;++x)a[x-lo]=(lo<=x-d&&x-d<=hi)?old[x-d-lo]:inf;}
   verify(f,a);++operations;
  }
  f.clear();assert(f.empty()&&f.min_value()==0&&!f.argmin().lower&&!f.argmin().upper);
 }
 SlopeTrick self;self.add_abs(7);self.shift(-3);self.add_constant(-10);self.merge(self);assert(self.eval(4)==-20&&self.eval(0)==-12&&self.size()==4);
 const I mn=std::numeric_limits<I>::min(),mx=std::numeric_limits<I>::max();
 SlopeTrick extreme;extreme.add_abs(mn);assert(extreme.argmin().lower==mn&&extreme.argmin().upper==mn&&extreme.eval(mn)==0&&extreme.eval(mn+1)==1);
 bool caught=false;try{extreme.eval(mx);}catch(const std::overflow_error&){caught=true;}assert(caught&&extreme.eval(mn)==0);
 caught=false;try{extreme.add_abs(mx);}catch(const std::overflow_error&){caught=true;}assert(caught&&extreme.size()==2&&extreme.eval(mn)==0);
 SlopeTrick shifted;shifted.add_abs(mx);shifted.shift(1);caught=false;try{shifted.argmin();}catch(const std::overflow_error&){caught=true;}assert(caught);shifted.shift(-1);assert(shifted.argmin().lower==mx);
 SlopeTrick offset;offset.add_abs(0);offset.shift(mx);caught=false;try{offset.shift(1);}catch(const std::overflow_error&){caught=true;}assert(caught&&offset.argmin().lower==mx);
 SlopeTrick constant;constant.add_constant(mn);constant.add_constant(mx);assert(constant.min_value()==-1);constant.add_constant(mx);caught=false;try{constant.add_constant(2);}catch(const std::overflow_error&){caught=true;}assert(caught&&constant.min_value()==mx-1);
#ifndef SKIP_RUNTIME_CHECKS
 caught=false;try{constant.shift(2,1);}catch(const std::invalid_argument&){caught=true;}assert(caught&&constant.min_value()==mx-1);
#endif
 // 直接128bit参照により符号境界近傍の単一hinge/absと平行移動を確認。
#if defined(__SIZEOF_INT128__)
 for(I anchor:std::vector<I>{mn,mn+100,-100,0,100,mx-100,mx})for(I delta:std::vector<I>{-100,-1,0,1,100}){
  __int128 x=static_cast<__int128>(anchor)+delta;if(x<mn||x>mx)continue;SlopeTrick f;f.add_abs(anchor);assert(f.eval(static_cast<I>(x))==std::abs(delta));
 }
#endif
 std::cout<<"slope reference operations="<<operations<<" integer boundaries OK\n";
}
