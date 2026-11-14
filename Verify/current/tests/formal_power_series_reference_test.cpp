// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef FPS_HEADER
#define FPS_HEADER "../../../Math/Formal_Power_Series.cpp"
#endif
#ifndef NTT_HEADER
#define NTT_HEADER "../../../Math/NTT.cpp"
#endif
#ifdef SIRKIT_TEST_NTT_FIRST
#include NTT_HEADER
#endif
#include FPS_HEADER
#include FPS_HEADER
#include NTT_HEADER
#include <cstdio>
#include <cstdlib>
#include <random>
namespace {
using V=std::vector<std::uint32_t>;using F=FormalPowerSeries;
constexpr std::uint32_t p=998244353;std::uint64_t checks=0;
void check(bool value){++checks;if(!value)std::abort();}
std::uint32_t product(std::uint32_t a,std::uint32_t b){return static_cast<std::uint32_t>(std::uint64_t{a}*b%p);}
std::uint32_t add(std::uint32_t a,std::uint32_t b){return a+b>=p?a+b-p:a+b;}
std::uint32_t sub(std::uint32_t a,std::uint32_t b){return a>=b?a-b:a+p-b;}
std::uint32_t power(std::uint32_t a,std::uint64_t k){std::uint32_t b=1;for(;k;k>>=1){if(k&1)b=product(a,b);a=product(a,a);}return b;}
V multiply(const V&a,const V&b,std::size_t n){V c(n);for(std::size_t i=0;i<a.size()&&i<n;++i)for(std::size_t j=0;j<b.size()&&j<n-i;++j)c[i+j]=add(c[i+j],product(a[i],b[j]));return c;}
V inverse(const V&a,std::size_t n){V b(n);if(n==0)return b;b[0]=power(a[0],p-2);for(std::size_t i=1;i<n;++i){std::uint32_t sum=0;for(std::size_t j=1;j<=i&&j<a.size();++j)sum=add(sum,product(a[j],b[i-j]));b[i]=product(sum==0?0:p-sum,b[0]);}return b;}
V exponential(const V&a,std::size_t n){V b(n);if(n==0)return b;b[0]=1;for(std::size_t i=1;i<n;++i){std::uint32_t sum=0;for(std::size_t j=1;j<=i&&j<a.size();++j)sum=add(sum,product(product(a[j],static_cast<std::uint32_t>(j)),b[i-j]));b[i]=product(sum,power(static_cast<std::uint32_t>(i),p-2));}return b;}
V logarithm(const V&a,std::size_t n){if(n==0)return{};V derivative(a.empty()?0:a.size()-1);for(std::size_t i=1;i<a.size();++i)derivative[i-1]=product(a[i],static_cast<std::uint32_t>(i));const auto mul=multiply(derivative,inverse(a,n),n-1);V b(n);for(std::size_t i=1;i<n;++i)b[i]=product(mul[i-1],power(static_cast<std::uint32_t>(i),p-2));return b;}
V pow_ref(V a,std::uint64_t k,std::size_t n){V b(n);if(n)b[0]=1;a.resize(n);for(;k;k>>=1){if(k&1)b=multiply(b,a,n);a=multiply(a,a,n);}return b;}
void trim(V&a){while(!a.empty()&&a.back()==0)a.pop_back();}
std::pair<V,V> divide(V a,V b){trim(a);trim(b);if(a.size()<b.size())return{{},a};V q(a.size()-b.size()+1);const auto inv=power(b.back(),p-2);for(std::size_t end=q.size();end;--end){const auto i=end-1;q[i]=product(a[i+b.size()-1],inv);for(std::size_t j=0;j<b.size();++j)a[i+j]=sub(a[i+j],product(q[i],b[j]));}a.resize(b.size()-1);trim(a);trim(q);return{q,a};}
template<class E,class G>void rejects(G g){bool yes=false;try{g();}catch(const E&){yes=true;}check(yes);}
}
int main(){std::mt19937_64 rng(0xf053646f626eULL);
for(int rep=0;rep<500;++rep){const std::size_t n=rng()%180;V a(n);for(auto&x:a)x=static_cast<std::uint32_t>(rng()%p);if(n&&a[0]==0)a[0]=1;const F f(a);check(f.inverse(n).coefficients()==inverse(a,n));V unit=a;if(n)unit[0]=1;const F u(unit);check(u.log(n).coefficients()==logarithm(unit,n));check(u.log(n).exp(n)==u.prefix(n));V zero=a;if(n)zero[0]=0;check(F(zero).exp(n).coefficients()==exponential(zero,n));const auto derivative=f.derivative();V d(n?n-1:0);for(std::size_t i=1;i<n;++i)d[i-1]=product(a[i],static_cast<std::uint32_t>(i));check(derivative.coefficients()==d);V integral(n+1);for(std::size_t i=0;i<n;++i)integral[i+1]=product(a[i],power(static_cast<std::uint32_t>(i+1),p-2));check(f.integral().coefficients()==integral);
const std::uint64_t k=rep%9;check(f.pow(k,n).coefficients()==pow_ref(a,k,n));if(n<35)check(f.pow(UINT64_MAX,n).coefficients()==pow_ref(a,UINT64_MAX,n));const auto squared=multiply(a,a,n);const auto root=F(squared).sqrt(n);check(root.has_value());check(multiply(root->coefficients(),root->coefficients(),n)==squared);if(n&&a[0]!=0)check(root->get(0)==std::min(a[0],p-a[0]));
V b(rng()%120+1);for(auto&x:b)x=static_cast<std::uint32_t>(rng()%p);if(b.back()==0)b.back()=1;const auto expected=divide(a,b);const auto [q,r]=f.divmod(F(b));check(q.coefficients()==expected.first&&r.coefficients()==expected.second);check((f/F(b)).coefficients()==expected.first);check((f%F(b)).coefficients()==expected.second);auto overwritten=f;overwritten/=F(b);check(overwritten.coefficients()==expected.first);overwritten=f;overwritten%=F(b);check(overwritten.coefficients()==expected.second);const auto saved=f;auto copy=f;copy+=copy;V twice=a;for(auto&x:twice)x=add(x,x);check(copy.coefficients()==twice);copy=f;copy-=copy;check(copy.coefficients()==V(n));copy=f;copy*=copy;const auto full=n?multiply(a,a,n*2-1):V{};check(copy.coefficients()==full);check(saved==f);
}
for(int leading=0;leading<18;++leading)for(int degree=0;degree<38;++degree){V a(static_cast<std::size_t>(leading)+5);a[leading]=3;a[leading+1]=2;a[leading+2]=4;const F f(a);for(std::uint64_t k:{0,1,2,3,9})check(f.pow(k,degree).coefficients()==pow_ref(a,k,degree));const auto squared=multiply(a,a,degree);const auto root=F(squared).sqrt(degree);check(root.has_value());check(multiply(root->coefficients(),root->coefficients(),degree)==squared);}
for(std::uint32_t value=0;value<200;++value){const auto square=product(value,value);const auto root=F(V{square}).sqrt(7);check(root&&root->get(0)==value);check(multiply(root->coefficients(),root->coefficients(),7)==V({square,0,0,0,0,0,0}));}check(!F{3}.sqrt(1));check(!F{0,1}.sqrt(2));check(F{0,0,1}.sqrt(3)->coefficients()==V({0,1,0}));check(F{}.pow(0,4).coefficients()==V({1,0,0,0}));check(F{0,0,1}.pow(UINT64_MAX,500).coefficients()==V(500));check(F{}.exp(5).coefficients()==V({1,0,0,0,0}));check(F{2}.log(0).empty());check(F{1}.exp(0).empty());check(F{}.inverse(0).empty());check(F{}.sqrt(0)->empty());
const std::size_t large=4096;const auto ones=F{1,-1}.inverse(large);check(ones.coefficients()==V(large,1));const auto logarithm_one_plus=F{1,1}.log(large);for(std::size_t i=1;i<large;++i){const auto inverse_i=power(static_cast<std::uint32_t>(i),p-2);check(logarithm_one_plus.get(i)==(i&1?inverse_i:p-inverse_i));}const auto exponential_x=F{0,1}.exp(large);std::uint32_t factorial=1;for(std::size_t i=0;i<large;++i){if(i)factorial=product(factorial,static_cast<std::uint32_t>(i));check(exponential_x.get(i)==power(factorial,p-2));}
F value(std::vector<std::int64_t>{INT64_MIN,-1,0,INT64_MAX});check(value[1]==p-1);value.set(1,UINT64_MAX);check(value[1]==UINT64_MAX%p);value.resize(8);check(value.get(100)==0);value.trim();check(value.size()==4);check(F{0}!=F{});auto moved=std::move(value);value.resize(3);value.set(0,1);check(value.inverse(3).coefficients()==V({1,0,0}));check(moved.size()==4);
rejects<std::domain_error>([]{(void)F{}.inverse(1);});rejects<std::domain_error>([]{(void)F{0}.inverse();});rejects<std::domain_error>([]{(void)F{1}.divmod(F{0,0});});rejects<std::length_error>([]{(void)F{1}.inverse(F::max_precision()+1);});rejects<std::length_error>([]{(void)F{0}.exp(F::max_precision()+1);});
#ifndef SKIP_RUNTIME_CHECKS
rejects<std::domain_error>([]{(void)F{2}.log(1);});rejects<std::domain_error>([]{(void)F{1}.exp(1);});rejects<std::out_of_range>([]{(void)F{1}[2];});rejects<std::out_of_range>([]{F a{1};a.set(2,0);});
#endif
F::clear_cache();check(F{1,1}.integral().coefficients()==V({0,1,499122177}));F::clear_cache();std::printf("FPS reference checks=%ju\n",static_cast<std::uintmax_t>(checks));}
