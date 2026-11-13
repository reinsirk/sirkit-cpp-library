// SPDX-License-Identifier: CC0-1.0
#ifndef RECURRENCE_HEADER
#define RECURRENCE_HEADER "../../../Math/Linear_Recurrence.cpp"
#endif
#ifndef BM_HEADER
#define BM_HEADER "../../../Math/Berlekamp_Massey.cpp"
#endif
#include BM_HEADER
#include RECURRENCE_HEADER
#include BM_HEADER
#include RECURRENCE_HEADER
#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>
static std::uint64_t checks=0;
void require(bool x){++checks;if(!x)throw std::runtime_error("linear recurrence reference mismatch");}
template<std::uint32_t Mod>struct F {
 std::uint32_t value=0;
 F()=default;F(std::uint64_t x):value(static_cast<std::uint32_t>(x%Mod)){}
 F&operator+=(F x){value=static_cast<std::uint32_t>((std::uint64_t{value}+x.value)%Mod);return *this;}
 F&operator-=(F x){value=static_cast<std::uint32_t>((std::uint64_t{value}+Mod-x.value)%Mod);return *this;}
 F&operator*=(F x){value=static_cast<std::uint32_t>(std::uint64_t{value}*x.value%Mod);return *this;}
 friend F operator+(F a,F b){return a+=b;}friend F operator-(F a,F b){return a-=b;}friend F operator*(F a,F b){return a*=b;}
 friend bool operator==(F,F)=default;
 friend F operator/(F a,F b){require(b.value!=0);F result{1};for(std::uint32_t e=Mod-2;e;e>>=1,b*=b)if(e&1)result*=b;return a*result;}
};
struct GF4 {
 std::uint32_t value=0;GF4()=default;GF4(std::uint32_t x):value(x%4){}
 GF4&operator+=(GF4 b){value^=b.value;return *this;}GF4&operator-=(GF4 b){return *this+=b;}
 GF4&operator*=(GF4 b){const auto lo=(value&1)*(b.value&1),middle=((value&1)*(b.value>>1))^((value>>1)*(b.value&1)),hi=(value>>1)*(b.value>>1);value=(lo^hi)|((middle^hi)<<1);return *this;}
 friend GF4 operator+(GF4 a,GF4 b){return a+=b;}friend GF4 operator-(GF4 a,GF4 b){return a-=b;}friend GF4 operator*(GF4 a,GF4 b){return a*=b;}
 friend bool operator==(GF4,GF4)=default;friend GF4 operator/(GF4 a,GF4 b){require(b.value!=0);return a*(b*b);}
};
template<class T>std::size_t smallest_order(const std::vector<T>&s){
 for(std::size_t d=0;d<=s.size();++d){std::vector<std::vector<T>>matrix(s.size()-d,std::vector<T>(d+1,T{0}));for(std::size_t i=d;i<s.size();++i){for(std::size_t j=0;j<d;++j)matrix[i-d][j]=s[i-j-1];matrix[i-d][d]=s[i];}std::size_t pivot=0;
  for(std::size_t col=0;col<d&&pivot<matrix.size();++col){std::size_t row=pivot;while(row<matrix.size()&&matrix[row][col]==T{0})++row;if(row==matrix.size())continue;std::swap(matrix[row],matrix[pivot]);const auto inverse=T{1}/matrix[pivot][col];for(std::size_t j=col;j<=d;++j)matrix[pivot][j]*=inverse;for(std::size_t i=0;i<matrix.size();++i)if(i!=pivot){const auto factor=matrix[i][col];for(std::size_t j=col;j<=d;++j)matrix[i][j]-=factor*matrix[pivot][j];}++pivot;}
  bool valid=true;for(const auto&row:matrix){bool zero=true;for(std::size_t j=0;j<d;++j)zero&=row[j]==T{0};if(zero&&row[d]!=T{0})valid=false;}if(valid)return d;
 }std::abort();
}
template<class T>void check_bm(const std::vector<T>&s){const auto c=BerlekampMassey(s);require(c.size()==smallest_order(s));for(std::size_t i=c.size();i<s.size();++i){T value{0};for(std::size_t j=0;j<c.size();++j)value+=c[j]*s[i-j-1];require(value==s[i]);}}
template<class T>std::vector<T> naive(const std::vector<T>&a,const std::vector<T>&c,std::size_t n){std::vector<T>s(n,T{0});for(std::size_t i=0;i<std::min(n,a.size());++i)s[i]=a[i];for(std::size_t i=a.size();i<n;++i)for(std::size_t j=0;j<c.size();++j)s[i]+=c[j]*s[i-j-1];return s;}
template<class T>T matrix_term(const std::vector<T>&a,const std::vector<T>&c,std::uint64_t k){
 if(a.empty())return T{0};
 const auto d=a.size();std::vector<std::vector<T>>power(d,std::vector<T>(d,T{0})),acc(d,std::vector<T>(d,T{0}));for(std::size_t i=0;i<d;++i){power[0][i]=c[i];acc[i][i]=T{1};if(i)power[i][i-1]=T{1};}
 auto multiply=[&](const auto&x,const auto&y){auto z=std::vector(d,std::vector<T>(d,T{0}));for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j)for(std::size_t p=0;p<d;++p)z[i][j]+=x[i][p]*y[p][j];return z;};if(k<d)return a[static_cast<std::size_t>(k)];k-=d-1;while(k){if(k&1)acc=multiply(acc,power);power=multiply(power,power);k>>=1;}T value{0};for(std::size_t j=0;j<d;++j)value+=acc[0][j]*a[d-1-j];return value;
}
template<class T>void recurrence_cases(std::mt19937_64&rng,std::uint32_t mod){
 for(int rep=0;rep<150;++rep){const std::size_t d=rng()%9;std::vector<T>a(d),c(d);for(auto&x:a)x=T(rng()%mod);for(auto&x:c)x=T(rng()%mod);const auto s=naive(a,c,90);require(s==LinearRecurrencePrefix(a,c,90));for(std::uint64_t k:{UINT64_C(0),UINT64_C(1),UINT64_C(7),UINT64_C(64),UINT64_C(89),UINT64_C(123456789012345),UINT64_MAX}){auto term=LinearRecurrenceTerm(a,c,k);require(term==matrix_term(a,c,k));const auto w=LinearRecurrenceWeights(c,k);T dot{0};for(std::size_t i=0;i<d;++i)dot+=w[i]*a[i];require(dot==term);}}
}
int main(){
 std::mt19937_64 rng(0x739abc);for(std::size_t n=0;n<=10;++n)for(std::uint32_t bits=0;bits<(1u<<n);++bits){std::vector<F<2>>s(n);for(std::size_t j=0;j<n;++j)s[j]=F<2>((bits>>j)&1);check_bm(s);}
 for(int rep=0;rep<1200;++rep){const auto n=static_cast<std::size_t>(rng()%20);std::vector<GF4>s(n);for(auto&x:s)x=GF4(static_cast<std::uint32_t>(rng()%4));check_bm(s);std::vector<F<7>>t(n);for(auto&x:t)x=F<7>(rng()%7);check_bm(t);}
 recurrence_cases<F<2>>(rng,2);recurrence_cases<GF4>(rng,4);recurrence_cases<F<8>>(rng,8);recurrence_cases<F<65537>>(rng,65537);recurrence_cases<F<998244353>>(rng,998244353);
 for(int rep=0;rep<600;++rep){const std::size_t d=rng()%65;std::vector<std::uint32_t>a(d),c(d);for(auto&x:a)x=static_cast<std::uint32_t>(rng());for(auto&x:c)x=static_cast<std::uint32_t>(rng());std::vector<F<998244353>>aa,cc;for(auto x:a)aa.emplace_back(x);for(auto x:c)cc.emplace_back(x);const auto k=rep%3?static_cast<std::uint64_t>(rng()%400):rng();require(LinearRecurrenceTerm(a,c,k)==LinearRecurrenceTerm(aa,cc,k).value);const auto prefix=LinearRecurrencePrefix(a,c,100);const auto oracle=naive(aa,cc,100);for(std::size_t i=0;i<100;++i)require(prefix[i]==oracle[i].value);const auto w=LinearRecurrenceWeights(c,k);const auto ww=LinearRecurrenceWeights(cc,k);for(std::size_t i=0;i<d;++i)require(w[i]==ww[i].value);}
 require(BerlekampMassey(std::vector<std::uint32_t>{}).empty());require(BerlekampMassey(std::vector<std::uint32_t>(200)).empty());require(BerlekampMassey(std::vector<std::uint32_t>{1,0,0})==std::vector<std::uint32_t>{0});
#ifndef SKIP_RUNTIME_CHECKS
 bool caught=false;try{(void)LinearRecurrenceTerm(std::vector<std::uint32_t>{1},std::vector<std::uint32_t>{},2);}catch(const std::invalid_argument&){caught=true;}require(caught);
#endif
 std::cout<<"PASS recurrence checks="<<checks<<"; Gaussian minimum-order, GF4, ring8, matrix-power uint64, unnormalized residues\n";
}
