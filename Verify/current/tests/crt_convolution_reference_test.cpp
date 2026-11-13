#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#ifndef HEADER
#define HEADER "../../../Math/Convolution.cpp"
#endif
#include HEADER
using boost::multiprecision::cpp_int;
static std::uint64_t checks=0;
static void check(bool okay){++checks;if(!okay)throw std::runtime_error("independent convolution mismatch");}
template<class A,class B>
static std::vector<cpp_int> oracle(const std::vector<A>& a,const std::vector<B>& b){
 if(a.empty()||b.empty())return {};
 std::vector<cpp_int> out(a.size()+b.size()-1);
 for(std::size_t i=0;i<a.size();++i)for(std::size_t j=0;j<b.size();++j)out[i+j]+=cpp_int{a[i]}*b[j];
 return out;
}
template<class A,class B>
static void modular_check(CRTConvolution& engine,const std::vector<A>& a,const std::vector<B>& b,std::uint32_t p){
 auto expected=oracle(a,b);auto actual=engine.modular(a,b,p);check(actual.size()==expected.size());
 for(std::size_t i=0;i<actual.size();++i){cpp_int r=expected[i]%p;if(r<0)r+=p;check(actual[i]==r);}
}
static void integer_check(CRTConvolution& engine,const std::vector<std::int64_t>& a,const std::vector<std::int64_t>& b){
 auto expected=oracle(a,b);auto actual=engine.integer(a,b);check(actual.size()==expected.size());
 for(std::size_t i=0;i<actual.size();++i)check(actual[i]==expected[i]);
}
int main(){
 CRTConvolution engine;std::mt19937_64 random(9210234);
 constexpr auto lo=std::numeric_limits<std::int64_t>::min(),hi=std::numeric_limits<std::int64_t>::max();
 for(auto p:{1u,2u,4u,167772161u,469762049u,998244353u,1000000007u,2147483647u,4294967291u,4294967295u}){
  modular_check(engine,std::vector<std::int64_t>{lo,-1,0,1,hi},std::vector<std::uint64_t>{0,1,UINT64_MAX},p);
  modular_check(engine,std::vector<int>{},std::vector<int>{1},p);
  for(std::size_t n:{1,2,31,32,33,63,64,65,129})for(std::size_t m:{1,2,32,33,65}){
   std::vector<std::uint64_t>a(n),b(m);for(auto&v:a)v=random();for(auto&v:b)v=random();modular_check(engine,a,b,p);
   if(n==m){check(engine.modular_square(a,p)==engine.modular(a,a,p));}
  }
 }
 for(int rep=0;rep<250;++rep){
  const auto n=static_cast<std::size_t>(random()%100),m=static_cast<std::size_t>(random()%100);
  std::vector<std::int64_t>a(n),b(m);for(auto&v:a)v=static_cast<std::int64_t>(random()%2000001)-1000000;for(auto&v:b)v=static_cast<std::int64_t>(random()%2000001)-1000000;
  integer_check(engine,a,b);check(engine.integer_square(a)==engine.integer(a,a));
 }
 integer_check(engine,{lo},{1});integer_check(engine,{hi},{1});integer_check(engine,{lo,hi},{0,1});
 integer_check(engine,{lo,hi},{1,1});integer_check(engine,{},{hi});
 // (1-x)^64(1+x)^64=(1-x²)^64: 個々の積は128bitに迫るが、最終係数はint64内。
 std::vector<std::int64_t>a(65),b(65);cpp_int choose=1;
 for(int i=0;i<=64;++i){b[i]=choose.convert_to<std::int64_t>();a[i]=i%2?-b[i]:b[i];if(i<64)choose=choose*(64-i)/(i+1);}
 integer_check(engine,a,b);
 bool threw=false;try{(void)engine.integer({lo},{-1});}catch(const std::overflow_error&){threw=true;}check(threw);
 threw=false;try{(void)engine.modular(std::vector<int>{},std::vector<int>{},0);}catch(const std::invalid_argument&){threw=true;}check(threw);
 threw=false;try{engine.prepare(3);}catch(const std::length_error&){threw=true;}check(threw);
 threw=false;try{engine.prepare(CRTConvolution::max_length()+1);}catch(const std::length_error&){threw=true;}check(threw);
 engine.prepare(1024);const auto held=engine.roots_capacity();check(held>0);auto copied=engine;engine.clear_cache();check(engine.roots_capacity()==0&&copied.roots_capacity()==held);
 auto moved=std::move(copied);modular_check(moved,std::vector<int>(100,17),std::vector<int>(90,-3),1000000007);copied.prepare(64);check(copied.roots_capacity()>0);
 check(ConvolutionAnyMod(std::vector<int>{1,2},std::vector<int>{3,4},7)==std::vector<std::uint32_t>({3,3,1}));
 check(ConvolutionInt64({-1,2},{3,-4})==std::vector<std::int64_t>({-3,10,-8}));
 // portable limb multiplicationを独立多倍長整数で比較。
 for(int rep=0;rep<20000;++rep){const auto x=random();const auto y=static_cast<std::uint32_t>(random());const auto add=random();const auto got=reinsirk::detail::convolution_wide_product(x,y,add);const cpp_int expected=cpp_int{x}*y+add;check(cpp_int{got.high}*(cpp_int{1}<<64)+got.low==expected);}
 std::cout<<"PASS CRT convolution checks="<<checks<<"\n";
}
