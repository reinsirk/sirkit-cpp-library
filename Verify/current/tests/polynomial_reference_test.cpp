namespace sirkit {}
using namespace sirkit;
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>
#ifndef HEADER
#define HEADER "../../../Math/Polynomial.cpp"
#endif
#include HEADER
constexpr std::uint32_t prime=998244353;
static std::uint64_t checks=0;
static void check(bool okay){++checks;if(!okay)throw std::runtime_error("polynomial independent mismatch");}
static std::uint32_t mul(std::uint32_t a,std::uint32_t b){return static_cast<std::uint32_t>(std::uint64_t{a}*b%prime);}
static std::uint32_t power(std::uint32_t a,std::uint32_t e){std::uint32_t r=1;while(e){if(e&1)r=mul(r,a);a=mul(a,a);e/=2;}return r;}
static std::uint32_t eval(const std::vector<std::uint32_t>& f,std::uint32_t x){std::uint32_t r=0;for(auto i=f.size();i;--i)r=static_cast<std::uint32_t>((std::uint64_t{r}*x+f[i-1])%prime);return r;}
static std::vector<std::uint32_t> multiply(const std::vector<std::uint32_t>&a,const std::vector<std::uint32_t>&b){if(a.empty()||b.empty())return {};std::vector<std::uint32_t>r(a.size()+b.size()-1);for(std::size_t i=0;i<a.size();++i)for(std::size_t j=0;j<b.size();++j)r[i+j]=static_cast<std::uint32_t>((r[i+j]+std::uint64_t{a[i]}*b[j])%prime);return r;}
static std::vector<std::uint32_t> lagrange(const std::vector<std::uint32_t>&x,const std::vector<std::uint32_t>&y){
 std::vector<std::uint32_t>r(x.size());
 for(std::size_t i=0;i<x.size();++i){std::vector<std::uint32_t>term{1};std::uint32_t denominator=1;
  for(std::size_t j=0;j<x.size();++j)if(i!=j){term=multiply(term,{x[j]?prime-x[j]:0,1});denominator=mul(denominator,static_cast<std::uint32_t>((std::uint64_t{x[i]}+prime-x[j])%prime));}
  const auto factor=mul(y[i],power(denominator,prime-2));for(std::size_t j=0;j<term.size();++j)r[j]=static_cast<std::uint32_t>((r[j]+std::uint64_t{term[j]}*factor)%prime);
 }return r;
}
static std::vector<std::uint32_t> shifted(const std::vector<std::uint32_t>&f,std::uint32_t c){
 std::vector<std::uint32_t>out(f.size()),binomial{1};
 for(std::size_t i=0;i<f.size();++i){
  std::uint32_t cp=1;for(std::size_t j=i+1;j;--j){const auto k=j-1;out[k]=static_cast<std::uint32_t>((out[k]+std::uint64_t{f[i]}*mul(binomial[k],cp))%prime);cp=mul(cp,c);}
  binomial.push_back(1);for(std::size_t j=binomial.size()-1;j>1;--j)binomial[j-1]=(binomial[j-1]+binomial[j-2])%prime;
 }return out;
}
int main(){
 std::mt19937_64 rng(781222);ProductTree empty;check(empty.empty()&&empty.polynomial().coefficients()==std::vector<std::uint32_t>{1});check(empty.evaluate(std::vector<int>{1,2}).empty());check(empty.interpolate(std::vector<int>{}).empty());
 for(std::size_t n:{0,1,2,3,7,17,31,32,33,63,64,65,129}){
  std::set<std::uint32_t>seen;std::vector<std::uint32_t>x;while(x.size()<n){const auto value=static_cast<std::uint32_t>(rng()%prime);if(seen.insert(value).second)x.push_back(value);}
  ProductTree tree(x);std::vector<std::uint32_t>expected_product{1};for(const auto v:x)expected_product=multiply(expected_product,{v?prime-v:0,1});check(tree.polynomial().coefficients()==expected_product);
  for(std::size_t d:{0,1,2,31,32,33,63,130}){
   std::vector<std::uint32_t>f(d);for(auto&v:f)v=static_cast<std::uint32_t>(rng()%prime);auto y=tree.evaluate(f);check(y.size()==x.size());for(std::size_t i=0;i<n;++i)check(y[i]==eval(f,x[i]));
   if(n<=33)check(tree.interpolate(y).coefficients()==lagrange(x,y));
  }
  std::vector<std::uint32_t>f(n);for(auto&v:f)v=static_cast<std::uint32_t>(rng()%prime);check(tree.interpolate(tree.evaluate(f)).coefficients()==f);
  check(MultipointEvaluate(f,x)==tree.evaluate(f));check(PolynomialInterpolate(x,tree.evaluate(f)).coefficients()==f);
  auto copied=tree;tree.assign(std::vector<int>{2,3});check(copied.points()==x);auto moved=std::move(copied);check(copied.empty());check(moved.points()==x);copied=tree;check(copied.points()==tree.points());copied=std::move(moved);check(moved.empty()&&copied.points()==x);auto* self=&copied;copied=*self;copied=std::move(*self);check(copied.points()==x);
 }
 for(std::size_t n:{0,1,2,3,31,32,33,64,65,129})for(auto c:{0u,1u,prime-1,192837465u}){
  std::vector<std::uint32_t>f(n);for(auto&v:f)v=static_cast<std::uint32_t>(rng()%prime);const auto actual=TaylorShift(f,c);check(actual.coefficients()==shifted(f,c));check(TaylorShift(actual,prime-c).coefficients()==f);
 }
 for(const auto& x:std::vector<std::vector<std::int64_t>>{{1,1},{0,998244353},{-1,998244352},{std::numeric_limits<std::int64_t>::min(),std::numeric_limits<std::int64_t>::min()}}){
  ProductTree tree(x);auto y=tree.evaluate(std::vector<int>{1,2,3});check(y[0]==y[1]);bool threw=false;try{(void)tree.interpolate(y);}catch(const std::domain_error&){threw=true;}check(threw);
 }
 const std::vector<std::uint64_t>wide{0,UINT64_MAX,UINT64_MAX-1,998244353};ProductTree tree(wide);
 for(std::size_t i=0;i<wide.size();++i)check(tree.points()[i]==wide[i]%prime);
 const auto values=tree.evaluate(std::vector<std::int64_t>{INT64_MIN,-1,0,INT64_MAX});
 std::vector<std::uint32_t>canonical;for(auto v:{INT64_MIN,std::int64_t{-1},std::int64_t{0},INT64_MAX}){auto r=v%static_cast<std::int64_t>(prime);canonical.push_back(static_cast<std::uint32_t>(r<0?r+prime:r));}
 for(std::size_t i=0;i<wide.size();++i)check(values[i]==eval(canonical,tree.points()[i]));
 check(TaylorShift(std::vector<int>{1,2,3},UINT64_MAX).coefficients()==shifted({1,2,3},static_cast<std::uint32_t>(UINT64_MAX%prime)));
#ifndef SKIP_RUNTIME_CHECKS
 bool threw=false;try{(void)ProductTree(std::vector<int>{1,2}).interpolate(std::vector<int>{1});}catch(const std::invalid_argument&){threw=true;}check(threw);
#endif
 std::cout<<"PASS polynomial checks="<<checks<<"; Horner/Lagrange/binomial, duplication, ownership, normalized wide inputs\n";
}
