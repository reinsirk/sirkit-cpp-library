// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_BITSET_SOURCE
#define SIRKIT_BITSET_SOURCE "../../../DataStructure/Dynamic_Bitset.cpp"
#endif
#include SIRKIT_BITSET_SOURCE
#include <array>
#include <cstdlib>
#include <iostream>
#include <random>
std::uint64_t checks=0;
void require(bool value){++checks;if(!value){std::cerr<<"failed check "<<checks<<'\n';std::abort();}}
template<class F>void rejected(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}require(caught);}
std::mt19937_64 random_engine(517736586196ULL);
std::size_t rng(std::size_t n){return static_cast<std::size_t>(random_engine()%n);}
std::string encode(const std::vector<bool>& a){std::string s;for(auto i=a.size();i;--i)s+=a[i-1]?'1':'0';return s;}
void compare(const DynamicBitset& b,const std::vector<bool>& a){
 require(b.size()==a.size());require(b.empty()==a.empty());require(b.to_string()==encode(a));require(b.count()==static_cast<std::size_t>(std::count(a.begin(),a.end(),true)));require(b.all()==std::all_of(a.begin(),a.end(),[](bool x){return x;}));require(b.any()==std::any_of(a.begin(),a.end(),[](bool x){return x;}));require(b.none()==std::none_of(a.begin(),a.end(),[](bool x){return x;}));
 require(b.words().size()==a.size()/64+(a.size()%64!=0));
 for(std::size_t i=0;i<a.size();++i){require(b.test(i)==a[i]);require(b[i]==a[i]);require(((b.words()[i/64]>>(i%64))&1)==a[i]);}
 if(a.size()%64)require((b.words().back()>>(a.size()%64))==0);
 std::size_t first=a.size(),last=a.size();for(std::size_t i=0;i<a.size();++i)if(a[i]){if(first==a.size())first=i;last=i;}
 require(b.find_first()==first);require(b.find_last()==last);
 std::vector<std::size_t> positions;for(auto p=b.find_first();p!=b.size();p=b.find_next(p)){require(p<a.size()&&a[p]);positions.push_back(p);require(positions.size()<=a.size());}
 require(positions.size()==b.count());
 auto previous=a.size();for(std::size_t p=0;p<=a.size();++p){require(b.find_prev(p)==previous);if(p<a.size()&&a[p])previous=p;} auto following=a.size();for(auto i=a.size()+1;i;--i){auto p=i-1;require(b.find_next(p)==following);if(p<a.size()&&a[p])following=p;}
 require(b.find_next(SIZE_MAX)==a.size());require(b.find_prev(SIZE_MAX)==last);require(b.count(0,a.size())==b.count());require(b.count(0,0)==0);require(b.count(a.size(),a.size())==0);
 for(int j=0;j<4;++j){auto l=rng(a.size()+1),r=rng(a.size()+1);if(l>r)std::swap(l,r);require(b.count(l,r)==static_cast<std::size_t>(std::count(a.begin()+static_cast<std::ptrdiff_t>(l),a.begin()+static_cast<std::ptrdiff_t>(r),true)));}
}
std::vector<bool> shift(const std::vector<bool>& a,std::size_t k,bool left){std::vector<bool>b(a.size());if(k>=a.size())return b;for(std::size_t i=0;i<a.size();++i){if(left){if(i>=k)b[i]=a[i-k];}else if(k<a.size()-i)b[i]=a[i+k];}return b;}
void shift_case(const DynamicBitset& b,const std::vector<bool>& a,std::size_t k){auto l=b;l<<=k;compare(l,shift(a,k,true));require(l==(b<<k));auto r=b;r>>=k;compare(r,shift(a,k,false));require(r==(b>>k));auto o=b;o.shift_or(k);auto expected=shift(a,k,true);for(std::size_t i=0;i<a.size();++i)expected[i]=expected[i]||a[i];compare(o,expected);}
void run_checks(){
 for(std::size_t n=0;n<=9;++n)for(std::uint64_t mask=0;mask<(std::uint64_t{1}<<n);++mask){std::vector<bool>a(n);DynamicBitset b(n);for(std::size_t i=0;i<n;++i)if((mask>>i)&1){a[i]=true;b.set(i);}compare(b,a);DynamicBitset from(encode(a));require(from==b);for(std::size_t k=0;k<=n+1;++k)shift_case(b,a,k);}
 const std::array<std::size_t,15> sizes{0,1,2,31,32,63,64,65,127,128,129,255,256,257,513};
 for(const auto n:sizes){for(bool value:{false,true}){DynamicBitset b(n,value);std::vector<bool>a(n,value);compare(b,a);for(const auto k:std::array<std::size_t,12>{0,1,2,31,63,64,65,127,128,n,n+1,SIZE_MAX})shift_case(b,a,k);}}
 for(int trial=0;trial<220;++trial){std::size_t n=sizes[rng(sizes.size())];DynamicBitset b(n);std::vector<bool>a(n);
  for(int step=0;step<180;++step){const auto op=rng(20);const auto p=n?rng(n):0;
   if(op==0&&n){const bool value=rng(2);auto* ptr=&b.set(p,value);a[p]=value;require(ptr==&b);}
   else if(op==1&&n){b.reset(p);a[p]=false;}
   else if(op==2&&n){b.flip(p);a[p]=!a[p];}
   else if(op==3){b.set();std::fill(a.begin(),a.end(),true);}
   else if(op==4){b.reset();std::fill(a.begin(),a.end(),false);}
   else if(op==5){b.flip();a.flip();}
   else if(op==6){const auto k=rng(700);b<<=k;a=shift(a,k,true);}
   else if(op==7){const auto k=rng(700);b>>=k;a=shift(a,k,false);}
   else if(op==8){const auto k=rng(700);b.shift_or(k);auto old=a;a=shift(old,k,true);for(std::size_t i=0;i<n;++i)a[i]=a[i]||old[i];}
   else if(op==9){const auto new_n=sizes[rng(sizes.size())];const bool value=rng(2);b.resize(new_n,value);a.resize(new_n,value);n=new_n;}
   else if(op==10){b.reserve_bits(rng(900));}
   else if(op==11&&n){const auto q=rng(n);b[p]=b[q];a[p]=a[q];b[p].flip();a[p]=!a[p];}
   else if(op==12&&n){const bool value=rng(2);b[p]=value;a[p]=value;b[p]^=value;a[p]=a[p]!=value;b[p]|=value;a[p]=a[p]||value;b[p]&=value;a[p]=a[p]&&value;require((~b[p])==!a[p]);}
   else if(op>=13&&op<=15){std::vector<bool>other(n);DynamicBitset c(n);for(std::size_t i=0;i<n;++i){other[i]=rng(2);c.set(i,other[i]);}std::size_t intersection=0;for(std::size_t i=0;i<n;++i)intersection+=a[i]&&other[i];require(b.intersection_count(c)==intersection);require(b.intersection_parity(c)==(intersection%2!=0));if(op==13){auto d=b&c;b&=c;require(d==b);for(std::size_t i=0;i<n;++i)a[i]=a[i]&&other[i];}if(op==14){auto d=b|c;b|=c;require(d==b);for(std::size_t i=0;i<n;++i)a[i]=a[i]||other[i];}if(op==15){auto d=b^c;b^=c;require(d==b);for(std::size_t i=0;i<n;++i)a[i]=a[i]!=other[i];}}
   else if(op==16){auto c=~b;auto expected=a;expected.flip();compare(c,expected);}
   else if(op==17){auto c=b;auto* alias=&c;c=*alias;c=std::move(*alias);require(c==b);auto d=std::move(c);require(c.empty()&&c.count()==0&&c.all());c.resize(65,true);require(c.count()==65);c=std::move(d);require(d.empty());require(c==b);swap(c,d);require(c.empty()&&d==b);}
   else if(op==18){require(b.intersection_count(b)==b.count());require(b.intersection_parity(b)==(b.count()%2!=0));auto c=b;c|=c;require(c==b);c&=c;require(c==b);c^=c;require(c.none());}
   else if(op==19){b.clear();a.clear();n=0;}
   compare(b,a);
  }
 }
 // 長いword列と非word境界の末尾。
 DynamicBitset large(1000003,true);require(large.count()==1000003);require(large.count(17,999999)==999982);large.resize(64);large.resize(1000003);require(large.count()==64);large.shift_or(999999);require(large.count()==68);
 DynamicBitset pa(20001),pb(20001);for(std::size_t i=0;i<20001;++i){if(i%3==0)pa.set(i);if(i%5==0)pb.set(i);}require(pa.count()==6667);require(pb.count()==4001);require(pa.intersection_count(pb)==1334);require(!pa.intersection_parity(pb));pb.set(19998);require(pa.intersection_count(pb)==1335);require(pa.intersection_parity(pb));require(pa.intersection_parity(pa)==(pa.count()%2!=0));
 require(!(DynamicBitset(1)==DynamicBitset(2)));require(DynamicBitset(1)!=DynamicBitset(2));
#ifndef SKIP_RUNTIME_CHECKS
 DynamicBitset b(65,true),shorter(64);auto old=b;rejected([&]{b.test(65);});rejected([&]{b[65]=false;});rejected([&]{b.set(SIZE_MAX);});rejected([&]{b.flip(65);});rejected([&]{b.reset(65);});rejected([&]{b.count(5,4);});rejected([&]{b.count(0,66);});rejected([&]{b&=shorter;});rejected([&]{b|=shorter;});rejected([&]{b^=shorter;});rejected([&]{b.intersection_count(shorter);});rejected([&]{b.intersection_parity(shorter);});rejected([]{DynamicBitset bad("101x");});require(b==old);DynamicBitset c63(63,true),c64(64,true);rejected([&]{c63&=c64;});rejected([&]{c63|=c64;});rejected([&]{c63^=c64;});rejected([&]{c63.intersection_count(c64);});rejected([&]{c63.intersection_parity(c64);});require(c63.count()==63);
#endif
}
int main(){run_checks();std::cout<<"dynamic bitset independent checks="<<checks<<'\n';}
