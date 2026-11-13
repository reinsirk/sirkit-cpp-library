// SPDX-License-Identifier: CC0-1.0
#ifndef HEADER
#define HEADER "../../../DataStructure/implicit_treap.cpp"
#endif
#include HEADER
#include <cassert>
#include <iostream>
#include <numeric>
#include <string>
struct Txt {std::string s;};
Txt concatenate(Txt a,Txt b){a.s+=b.s;return a;}
Txt text_empty(){return{};}
Txt text_apply(int f,Txt x){for(char& c:x.s)c=char('a'+(c-'a'+f)%26);return x;}
int text_compose(int f,int g){return(f+g)%26;}
int text_id(){return 0;}
using Tree=indexed_implicit_treap<Txt,concatenate,text_empty,int,text_apply,text_compose,text_id>;
struct Item{int id;std::string s;};
std::string fold(const std::vector<Item>& a,int l,int r){std::string s;for(int i=l;i<r;++i)s+=a[i].s;return s;}
void verify(Tree& t,const std::vector<Item>& a){
 assert(t.size()==int(a.size()));assert(t.all_prod().s==fold(a,0,int(a.size())));
 // ID lookup first exercises ancestors while delayed flags may still be present.
 for(int i=0;i<int(a.size());++i){assert(t.position_of(a[i].id)==i);assert(t.get_id(i)==a[i].id);assert(t.get_by_id(a[i].id).s==a[i].s);}
 auto values=t.to_vector();assert(values.size()==a.size());for(int i=0;i<int(a.size());++i)assert(values[i].s==a[i].s);
 for(int i=0;i<=int(a.size());++i){assert(t.prod(0,i).s==fold(a,0,i));assert(t.prod(i,int(a.size())).s==fold(a,i,int(a.size())));}
}
int main(){std::uint64_t cases=0;
 for(int n=0;n<=32;++n)for(int phase=0;phase<2;++phase){
  Tree base;std::vector<Item>a;
  for(int i=0;i<n;++i){std::string s(1,char('a'+i%26));base.insert_at_with_id(i,3*i,{s});a.push_back({3*i,s});}
  if(phase&&n){base.apply(0,n,1);for(auto&x:a)x.s=text_apply(1,{x.s}).s;base.reverse(0,n);std::reverse(a.begin(),a.end());base.apply(n/3,2*n/3,2);for(int i=n/3;i<2*n/3;++i)a[i].s=text_apply(2,{a[i].s}).s;}
  for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){
   {Tree t=base;auto ref=a;t.reverse(l,r);std::reverse(ref.begin()+l,ref.begin()+r);verify(t,ref);++cases;}
   {Tree t=base;auto ref=a;auto expected=fold(a,l,r);auto actual=t.erase_prod(l,r);assert(actual.s==expected);for(int i=l;i<r;++i)assert(!t.contains_id(a[i].id));ref.erase(ref.begin()+l,ref.begin()+r);verify(t,ref);++cases;}
   for(int m=l;m<=r;++m){Tree t=base;auto ref=a;t.rotate(l,m,r);std::rotate(ref.begin()+l,ref.begin()+m,ref.begin()+r);verify(t,ref);++cases;}
  }
 }
 std::cout<<cases<<" exhaustive noncommutative reversal/rotation/erase cases passed\n";
}
