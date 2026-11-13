// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_SEGMENT_TEST_SOURCE
#define SIRKIT_SEGMENT_TEST_SOURCE "../../../DataStructure/Segment_Tree.cpp"
#endif
#include SIRKIT_SEGMENT_TEST_SOURCE
#include <bit>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <string>
#include <vector>
std::uint64_t checks=0;
void check(bool x){++checks;if(!x)std::abort();}
struct Word {
 std::string text;
 Word()=delete;
 explicit Word(std::string value):text(std::move(value)){}
};
Word word_op(const Word& a,const Word& b){return Word(a.text+b.text);}
Word word_e(){return Word("");}
bool bool_op(bool a,bool b){return a&&b;}
bool bool_e(){return true;}
void word_case(int n){
 std::vector<Word> a;for(int i=0;i<n;++i)a.emplace_back(std::to_string(i)+",");
 segtree<Word,word_op,word_e> tree(a);
 for(int l=0;l<=n;++l){std::string expected;for(int r=l;r<=n;++r){check(tree.prod(l,r).text==expected);if(r<n)expected+=a[r].text;}}
 if(n){tree.set(n/2,Word("changed,"));a[n/2]=Word("changed,");for(int l=0;l<=n;++l){std::string expected;for(int r=l;r<=n;++r){check(tree.prod(l,r).text==expected);if(r<n)expected+=a[r].text;}}}
 auto copied=tree;auto moved=std::move(copied);check(copied.prod(0,0).text.empty());check(moved.prod(0,n).text==tree.prod(0,n).text);
}
void bool_case(int n){
 std::vector<bool>a(n,true);for(int i=2;i<n;i+=7)a[i]=false;
 segtree<bool,bool_op,bool_e> tree(a);
 for(int l=0;l<=n;++l){bool expected=true;for(int r=l;r<=n;++r){check(tree.prod(l,r)==expected);if(r<n)expected=expected&&a[r];}}
}
struct Partition {
 std::array<unsigned,2*std::numeric_limits<unsigned>::digits> left{},right{};
 unsigned left_count=0,right_count=0,stop=0;
 void append_left(unsigned node){check(left_count<left.size());left[left_count++]=node;}
 void prepend_right(unsigned node){check(right_count<right.size());right[right_count++]=node;}
};
unsigned dispatch_coverage=0;
// 基準は下位bit走査や分割点公式を使わず、通常の親半減式で節点列を列挙する。
Partition parent_partition(unsigned first,unsigned last){
 Partition result;
 while(first<last){
  if(first&1)result.append_left(first++);
  if(last&1)result.prepend_right(--last);
  first>>=1;last>>=1;
 }
 return result;
}
Partition hybrid_partition(unsigned first,unsigned last){
 Partition result;
 auto node=[](unsigned x){check(x!=0);return x>>std::countr_zero(x);};
 auto left=[&]{result.append_left(node(first));first+=first&(~first+1);};
 auto right=[&]{result.prepend_right(node(last)-1);last&=last-1;};
 if(first==last){result.stop=first;return result;}
 if((first&(~first+1))>last-first){
  dispatch_coverage|=1;result.stop=first;
  do{right();}while(first<last);
  return result;
 }
 left();
 if(first==last){dispatch_coverage|=2;result.stop=first;return result;}
 const auto remaining=last-first;
 if(remaining<=32){
  dispatch_coverage|=4;
  if(remaining==31)dispatch_coverage|=8;
  if(remaining==32)dispatch_coverage|=16;
  while(first<last&&(first&(~first+1))<=last-first)left();
  result.stop=first;
  while(first<last)right();
  return result;
 }
 dispatch_coverage|=32;
 if(remaining==33)dispatch_coverage|=64;
 const unsigned split=last&(0U-std::bit_floor((first-1)^(last-1)));
 check(first<=split&&split<=last);result.stop=split;
 if(split==last){while(first<split)left();return result;}
 right();
 while(first<split&&last>split){left();right();}
 while(first<split)left();
 while(last>split)right();
 check(first==split&&last==split);
 return result;
}
void compare_hybrid_partition(unsigned base,unsigned n,unsigned l,unsigned r){
 using W=std::uint64_t;
 const W begin=W(base)+l,end=W(base)+r;
 check(begin<=end&&end<=std::numeric_limits<unsigned>::max());
 const auto reference=parent_partition(unsigned(begin),unsigned(end));
 const auto actual=hybrid_partition(unsigned(begin),unsigned(end));
 check(actual.left_count==reference.left_count&&actual.right_count==reference.right_count);
 W cursor=begin;
 auto consume=[&](unsigned node){
  check(node!=0&&W(node)<2*W(base));
  const unsigned depth=std::bit_width(node)-1;
  const W width=W(base)>>depth;
  check(width!=0&&W(node)*width==cursor);
  cursor+=width;check(cursor<=end);
 };
 for(unsigned i=0;i<reference.left_count;++i){check(actual.left[i]==reference.left[i]);consume(actual.left[i]);}
 check(cursor==actual.stop);
 for(unsigned i=reference.right_count;i>0;--i){check(actual.right[i-1]==reference.right[i-1]);consume(actual.right[i-1]);}
 check(cursor==end);
 check(actual.left_count+actual.right_count<=2*std::numeric_limits<unsigned>::digits);
 (void)n;
}
void geometry(unsigned base,unsigned n,unsigned l,unsigned r){
 using W=std::uint64_t;static_assert(std::numeric_limits<unsigned>::digits<std::numeric_limits<W>::digits);
 check(l<=r&&r<=n&&n<=unsigned(std::numeric_limits<int>::max()));
 compare_hybrid_partition(base,n,l,r);
 if(l==r)return;
 W fw=W(base)+l,lw=W(base)+r;check(lw<=std::numeric_limits<unsigned>::max());
 unsigned first=unsigned(fw),last=unsigned(lw);unsigned count=0;
 auto left_node=[&](unsigned x,unsigned step){check(x!=0);const unsigned index=x>>std::countr_zero(x);check(index>=1&&W(index)<W(base)*2);check(W(index)*step==x);check(W(x)+step<=lw);++count;};
 auto right_node=[&](unsigned x){check(x!=0);const unsigned step=x&(~x+1);check(step<=x-first);const unsigned index=(x>>std::countr_zero(x))-1;check(index>=1&&W(index)<W(base)*2);check(W(index)*step==W(x)-step);check(W(x)-step>=first);++count;};
 unsigned step=first&(~first+1);
 if(step<=last-first){left_node(first,step);first+=step;while(first<last&&(step=first&(~first+1))<=last-first){left_node(first,step);first+=step;}}
 if(first!=last){right_node(last);last-=last&(~last+1);while(first<last){right_node(last);last-=last&(~last+1);}}
 check(first==last);check(count<=2*std::numeric_limits<unsigned>::digits);
}
int main(){
 for(int n=0;n<=80;++n){word_case(n);bool_case(n);}
 for(int n:{127,128,129,255,256,257,511,512,513}){word_case(n);bool_case(n);}
 std::mt19937 random(0x781659u);
 for(unsigned k=0;k<unsigned(std::numeric_limits<unsigned>::digits-1)+1;++k){
  const unsigned base=unsigned(1)<<k;
  for(unsigned n:{base/2+1,base-1,base}){
   if(n==0||n>unsigned(std::numeric_limits<int>::max())||std::bit_ceil(n)!=base)continue;
   for(unsigned l:{0u,1u,n/2,n-1,n})for(unsigned r:{0u,1u,n/2,n-1,n})if(l<=r&&r<=n)geometry(base,n,l,r);
   for(int rep=0;rep<2000;++rep){unsigned l=random()%(n+1),r=random()%(n+1);if(l>r)std::swap(l,r);geometry(base,n,l,r);}
  }
 }
 // 初回の左節点を選んだ後の残り幅31/32/33を、閾値の両側で明示する。
 for(unsigned width:{32u,33u,34u})geometry(128,100,1,1+width);
 const unsigned large_base=unsigned(1)<<(std::numeric_limits<unsigned>::digits-1);
 const unsigned large_n=unsigned(std::numeric_limits<int>::max());
 for(unsigned width:{32u,33u,34u})geometry(large_base,large_n,large_n-100,large_n-100+width);
 check(dispatch_coverage==127);
 std::printf("greedy independent checks=%ju\n",std::uintmax_t(checks));
}
