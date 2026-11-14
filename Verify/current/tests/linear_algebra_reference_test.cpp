// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef HEADER
#define HEADER "../../../Math/Gaussian_Elimination.cpp"
#endif
#include HEADER
#include HEADER
#include <bit>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>
static std::uint64_t checks=0;
void require(bool value){++checks;if(!value)throw std::runtime_error("matrix reference mismatch");}
template<std::uint32_t Mod>struct F {
 std::uint32_t value=0;F()=default;F(std::uint64_t x):value(static_cast<std::uint32_t>(x%Mod)){}
 F&operator+=(F x){value=static_cast<std::uint32_t>((std::uint64_t{value}+x.value)%Mod);return *this;}
 F&operator-=(F x){value=static_cast<std::uint32_t>((std::uint64_t{value}+Mod-x.value)%Mod);return *this;}
 F&operator*=(F x){value=static_cast<std::uint32_t>(std::uint64_t{value}*x.value%Mod);return *this;}
 friend F operator+(F a,F b){return a+=b;}friend F operator-(F a,F b){return a-=b;}friend F operator*(F a,F b){return a*=b;}friend bool operator==(F,F)=default;
 friend F operator/(F a,F b){require(b.value!=0);F r{1};for(std::uint32_t e=Mod-2;e;e>>=1,b*=b)if(e&1)r*=b;return a*r;}
};
struct GF4 {
 std::uint32_t value=0;GF4()=default;GF4(std::uint32_t x):value(x%4){}
 GF4&operator+=(GF4 b){value^=b.value;return *this;}GF4&operator-=(GF4 b){return *this+=b;}
 GF4&operator*=(GF4 b){const auto lo=(value&1)*(b.value&1),middle=((value&1)*(b.value>>1))^((value>>1)*(b.value&1)),hi=(value>>1)*(b.value>>1);value=(lo^hi)|((middle^hi)<<1);return *this;}
 friend GF4 operator+(GF4 a,GF4 b){return a+=b;}friend GF4 operator-(GF4 a,GF4 b){return a-=b;}friend GF4 operator*(GF4 a,GF4 b){return a*=b;}friend bool operator==(GF4,GF4)=default;friend GF4 operator/(GF4 a,GF4 b){require(b.value!=0);return a*(b*b);}
};
template<class T>T permutation_determinant(const Matrix<T>&matrix){std::vector<std::size_t>p(matrix.rows());std::iota(p.begin(),p.end(),0);T result{0};do{T term{1};std::size_t inversions=0;for(std::size_t i=0;i<p.size();++i){term*=matrix(i,p[i]);for(std::size_t j=i+1;j<p.size();++j)inversions+=p[i]>p[j];}result+=(inversions&1)?T{0}-term:term;}while(std::next_permutation(p.begin(),p.end()));return result;}
template<class T>std::size_t minor_rank(const Matrix<T>&matrix){for(std::size_t k=std::min(matrix.rows(),matrix.cols());k>0;--k)for(std::uint32_t r=0;r<(1u<<matrix.rows());++r){if(std::popcount(r)!=static_cast<int>(k))continue;for(std::uint32_t c=0;c<(1u<<matrix.cols());++c){if(std::popcount(c)!=static_cast<int>(k))continue;Matrix<T>minor(k,k);std::size_t i=0;for(std::size_t row=0;row<matrix.rows();++row)if((r>>row)&1){std::size_t j=0;for(std::size_t col=0;col<matrix.cols();++col)if((c>>col)&1)minor(i,j++)=matrix(row,col);++i;}if(permutation_determinant(minor)!=T{0})return k;}}return 0;}
template<class T>Matrix<T> oracle_product(const Matrix<T>&a,const Matrix<T>&b){Matrix<T>result(a.rows(),b.cols());for(std::size_t i=0;i<a.rows();++i)for(std::size_t j=0;j<b.cols();++j)for(std::size_t k=0;k<a.cols();++k)result(i,j)+=a(i,k)*b(k,j);return result;}
template<class T>void product_cases(std::mt19937_64&rng,std::uint32_t mod){for(int rep=0;rep<250;++rep){const std::size_t m=rng()%7,k=rng()%7,n=rng()%7;Matrix<T>a(m,k),b(k,n);for(auto&x:a.values())x=T(rng()%mod);for(auto&x:b.values())x=T(rng()%mod);const auto c=a*b;require(c==oracle_product(a,b));require(c.transposed()==b.transposed()*a.transposed());require(a.transposed().transposed()==a);require(a+a-a==a);auto copy=a;copy*=b;require(copy==c);auto moved=std::move(copy);require(copy.rows()==0&&copy.cols()==0&&copy.empty());require(moved==c);copy=a;require(copy==a);auto flat=std::vector<T>(a.size(),T{0});for(std::size_t i=0;i<a.size();++i)flat[i]=a.values()[i];require(Matrix<T>(m,k,std::move(flat))==a);std::vector<T>x(k,T{1});const auto applied=MatrixApply(a,x);for(std::size_t i=0;i<m;++i){T sum{0};for(std::size_t j=0;j<k;++j)sum+=a(i,j);require(applied[i]==sum);}}}
template<class T>void field_cases(std::mt19937_64&rng,std::uint32_t mod){
 for(int rep=0;rep<200;++rep){const std::size_t m=rng()%5,n=rng()%5;Matrix<T>a(m,n);for(auto&x:a.values())x=T(rng()%mod);const auto expected=minor_rank(a);require(MatrixRank(a)==expected);auto e=GaussianElimination(a);auto r=GaussianElimination(a,true);require(e.rank()==expected&&r.rank()==expected);for(std::size_t i=0;i<expected;++i){const auto col=r.pivot_columns[i];require(r.matrix(i,col)==T{1});for(std::size_t j=0;j<m;++j)if(j!=i)require(r.matrix(j,col)==T{0});}
  std::vector<T>b(m,T{0});for(auto&x:b)x=T(rng()%mod);auto solution=SolveLinearSystem(a,b);std::size_t total=1;for(std::size_t j=0;j<n;++j)total*=mod;bool found=false;std::size_t count=0;for(std::size_t encoded=0;encoded<total;++encoded){auto code=encoded;std::vector<T>x(n,T{0});for(auto&v:x){v=T(code%mod);code/=mod;}if(MatrixApply(a,x)==b){found=true;++count;}}require(solution.has_value()==found);
  if(solution){require(MatrixApply(a,solution->particular)==b);require(solution->kernel.rows()==n-expected&&solution->kernel.cols()==n);require(minor_rank(solution->kernel)==n-expected);for(std::size_t i=0;i<solution->kernel.rows();++i){std::vector<T>v(n,T{0});for(std::size_t j=0;j<n;++j)v[j]=solution->kernel(i,j);require(MatrixApply(a,v)==std::vector<T>(m,T{0}));}std::size_t dimension=1;for(std::size_t i=0;i<n-expected;++i)dimension*=mod;require(count==dimension);}
 }
 for(std::size_t n=0;n<=5;++n)for(int rep=0;rep<50;++rep){Matrix<T>a(n,n);for(auto&x:a.values())x=T(rng()%mod);const auto det=permutation_determinant(a);require(MatrixDeterminant(a)==det);auto inv=MatrixInverse(a);require(inv.has_value()==(det!=T{0}));if(inv){require(a*(*inv)==MatrixIdentity<T>(n));require((*inv)*a==MatrixIdentity<T>(n));}for(std::uint64_t k=0;k<=5;++k){auto power=MatrixIdentity<T>(n);for(std::uint64_t j=0;j<k;++j)power=oracle_product(power,a);require(MatrixPower(a,k)==power);}}
}
int main(){std::mt19937_64 rng(61394);product_cases<F<8>>(rng,8);product_cases<F<998244353>>(rng,998244353);product_cases<GF4>(rng,4);field_cases<F<2>>(rng,2);field_cases<F<3>>(rng,3);field_cases<GF4>(rng,4);
 Matrix<F<7>>zero_columns(4,0);require(zero_columns.row(2).empty());require(MatrixRank(zero_columns)==0);require(SolveLinearSystem(zero_columns,std::vector<F<7>>(4)).has_value());require(!SolveLinearSystem(zero_columns,std::vector<F<7>>{0,0,1,0}).has_value());Matrix<F<7>>zero_rows(0,5);auto all=SolveLinearSystem(zero_rows,std::vector<F<7>>{});require(all&&all->kernel==MatrixIdentity<F<7>>(5));
 bool overflow=false;try{Matrix<F<7>>m(std::numeric_limits<std::size_t>::max(),2);(void)m;}catch(const std::length_error&){overflow=true;}require(overflow);
#ifndef SKIP_RUNTIME_CHECKS
 bool bad=false;try{Matrix<F<7>>m{{1,2},{3}};(void)m;}catch(const std::invalid_argument&){bad=true;}require(bad);bad=false;try{(void)(Matrix<F<7>>(2,3)*Matrix<F<7>>(2,2));}catch(const std::invalid_argument&){bad=true;}require(bad);bad=false;try{(void)zero_columns(0,0);}catch(const std::out_of_range&){bad=true;}require(bad);
#endif
 std::cout<<"PASS matrix checks="<<checks<<"; independent permutation/minor rank/brute solution; GF2 GF3 GF4 and ring8\n";
}
