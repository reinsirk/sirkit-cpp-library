// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_BINARY_MATRIX_SOURCE
#define SIRKIT_BINARY_MATRIX_SOURCE "../../../Math/Binary_Matrix.cpp"
#endif
#include SIRKIT_BINARY_MATRIX_SOURCE
#include <array>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
using Bytes=std::vector<std::vector<std::uint8_t>>;
std::uint64_t checks=0;std::mt19937_64 engine(149534745106ULL);
void require(bool value){++checks;if(!value){std::cerr<<"failed check "<<checks<<'\n';std::abort();}}
template<class F>void rejected(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}require(caught);}
std::size_t rng(std::size_t n){return static_cast<std::size_t>(engine()%n);}
BinaryMatrix packed(const Bytes&a,std::size_t cols){BinaryMatrix result(a.size(),cols);for(std::size_t i=0;i<a.size();++i)for(std::size_t j=0;j<cols;++j)result.set(i,j,a[i][j]);return result;}
Bytes unpacked(const BinaryMatrix&a){Bytes r(a.rows(),std::vector<std::uint8_t>(a.cols()));for(std::size_t i=0;i<a.rows();++i)for(std::size_t j=0;j<a.cols();++j)r[i][j]=a.test(i,j);return r;}
void compare(const BinaryMatrix&a,const Bytes&b,std::size_t cols){require(a.rows()==b.size()&&a.cols()==cols);require(a.empty()==(b.empty()||cols==0));for(std::size_t i=0;i<b.size();++i){auto words=a.row_words(i);require(words.size()==cols/64+(cols%64!=0));for(std::size_t j=0;j<cols;++j){require(a(i,j)==static_cast<bool>(b[i][j]));require(((words[j/64]>>(j%64))&1)==b[i][j]);}if(cols%64)require((words.back()>>(cols%64))==0);}}
std::size_t naive_rank(Bytes a,std::size_t cols){std::size_t rank=0;for(std::size_t col=0;col<cols&&rank<a.size();++col){std::size_t pivot=rank;while(pivot<a.size()&&!a[pivot][col])++pivot;if(pivot==a.size())continue;std::swap(a[pivot],a[rank]);for(auto row=rank+1;row<a.size();++row)if(a[row][col])for(std::size_t j=0;j<cols;++j)a[row][j]^=a[rank][j];++rank;}return rank;}
Bytes product(const Bytes&a,const Bytes&b,std::size_t k){Bytes result(a.size(),std::vector<std::uint8_t>(k));for(std::size_t i=0;i<a.size();++i)for(std::size_t j=0;j<k;++j)for(std::size_t x=0;x<b.size();++x)result[i][j]^=a[i][x]&b[x][j];return result;}
std::vector<std::uint8_t> apply(const Bytes&a,const std::vector<std::uint8_t>&x){std::vector<std::uint8_t> r(a.size());for(std::size_t i=0;i<a.size();++i)for(std::size_t j=0;j<x.size();++j)r[i]^=a[i][j]&x[j];return r;}
void solution_case(const BinaryMatrix&a,const Bytes&naive,std::size_t cols,const std::vector<std::uint8_t>&rhs,bool enumerate){
 const auto before=a;const auto answer=a.solve(rhs);require(a==before);const auto rank=naive_rank(naive,cols);
 std::set<std::vector<std::uint8_t>> exact;
 if(enumerate){for(std::uint64_t mask=0;mask<(std::uint64_t{1}<<cols);++mask){std::vector<std::uint8_t>x(cols);for(std::size_t j=0;j<cols;++j)x[j]=static_cast<std::uint8_t>((mask>>j)&1);if(apply(naive,x)==rhs)exact.insert(x);}require(answer.has_value()==!exact.empty());}
 if(!answer)return;
 require(answer->particular.size()==cols);require(apply(naive,answer->particular)==rhs);require(answer->kernel.rows()==cols-rank&&answer->kernel.cols()==cols);require(answer->pivot_columns.size()==rank);require(std::is_sorted(answer->pivot_columns.begin(),answer->pivot_columns.end()));auto kernel=unpacked(answer->kernel);require(naive_rank(kernel,cols)==cols-rank);
 for(const auto&row:kernel)require(apply(naive,row)==std::vector<std::uint8_t>(naive.size()));
 if(enumerate){std::set<std::vector<std::uint8_t>> actual;for(std::uint64_t mask=0;mask<(std::uint64_t{1}<<kernel.size());++mask){auto x=answer->particular;for(std::size_t i=0;i<kernel.size();++i)if((mask>>i)&1)for(std::size_t j=0;j<cols;++j)x[j]^=kernel[i][j];actual.insert(x);}require(actual==exact);std::vector<std::size_t> pivots;std::size_t old_rank=0;for(std::size_t prefix=1;prefix<=cols;++prefix){auto r=naive_rank(naive,prefix);if(r>old_rank)pivots.push_back(prefix-1);old_rank=r;}require(pivots==answer->pivot_columns);}
 auto copy=*answer;require(copy.kernel==answer->kernel&&copy.particular==answer->particular);auto* alias=&copy;copy=*alias;copy=std::move(*alias);auto moved=std::move(copy);require(copy.particular.empty()&&copy.kernel.rows()==0&&copy.kernel.cols()==0&&copy.pivot_columns.empty());copy=std::move(moved);require(moved.particular.empty()&&moved.kernel.empty()&&moved.pivot_columns.empty());require(copy.particular==answer->particular);
}
void small_exhaustive(){
 for(std::size_t r=0;r<=3;++r)for(std::size_t c=0;c<=3;++c)for(std::uint64_t mask=0;mask<(std::uint64_t{1}<<(r*c));++mask){
  Bytes naive(r,std::vector<std::uint8_t>(c));for(std::size_t i=0;i<r;++i)for(std::size_t j=0;j<c;++j)naive[i][j]=static_cast<std::uint8_t>((mask>>(i*c+j))&1);auto a=packed(naive,c);compare(a,naive,c);std::set<std::vector<std::uint8_t>> row_span;for(std::uint64_t subset=0;subset<(std::uint64_t{1}<<r);++subset){std::vector<std::uint8_t>x(c);for(std::size_t i=0;i<r;++i)if((subset>>i)&1)for(std::size_t j=0;j<c;++j)x[j]^=naive[i][j];row_span.insert(x);}std::size_t rank=0;while((std::size_t{1}<<rank)<row_span.size())++rank;require(a.rank()==rank);
  auto transposed=a.transpose();require(transposed.transpose()==a);for(std::size_t i=0;i<r;++i)for(std::size_t j=0;j<c;++j)require(transposed.test(j,i)==static_cast<bool>(naive[i][j]));
  for(std::uint64_t b=0;b<(std::uint64_t{1}<<r);++b){std::vector<std::uint8_t>rhs(r);for(std::size_t i=0;i<r;++i)rhs[i]=static_cast<std::uint8_t>((b>>i)&1);solution_case(a,naive,c,rhs,true);}
  if(r==c){require(a.determinant()==(rank==r));auto inv=a.inverse();require(inv.has_value()==(rank==r));if(inv){require(a**inv==BinaryMatrix::identity(r));require(*inv*a==BinaryMatrix::identity(r));}require(a.pow(0)==BinaryMatrix::identity(r));require(a.pow(1)==a);require(a.pow(2)==a*a);require(a.pow(3)==a*a*a);}
 }
}
void random_cases(){
 const std::array<std::size_t,12> sizes{0,1,2,3,31,63,64,65,127,128,129,257};
 for(int trial=0;trial<160;++trial){std::size_t r=rng(16),c=sizes[rng(sizes.size())];Bytes n(r,std::vector<std::uint8_t>(c));for(auto&row:n)for(auto&bit:row)bit=static_cast<std::uint8_t>(rng(2));auto a=packed(n,c);
  for(int step=0;step<35;++step){if(r){auto i=rng(r),j=rng(r);if(rng(3)==0){a.swap_rows(i,j);std::swap(n[i],n[j]);}else{auto start=rng(c+1);a.xor_rows(i,j,start);for(std::size_t col=start;col<c;++col)n[i][col]^=n[j][col];}if(c){auto col=rng(c);if(rng(2)){a.flip(i,col);n[i][col]^=1;}else{bool value=rng(2);a.set(i,col,value);n[i][col]=value;}}}compare(a,n,c);}
  require(a.rank()==naive_rank(n,c));auto b=a;require((a+b).rank()==0);require((a^b)==(a+b));b+=b;require(b==BinaryMatrix(r,c));b=a;b^=b;require(b==BinaryMatrix(r,c));require(a.transpose().transpose()==a);
  std::vector<std::uint8_t>x(c);for(auto&v:x)v=static_cast<std::uint8_t>(rng(2));auto rhs=apply(n,x);solution_case(a,n,c,rhs,c<=8);if(r){rhs[rng(r)]^=1;solution_case(a,n,c,rhs,c<=8);}
  auto copy=a;auto* alias=&copy;copy=*alias;copy=std::move(*alias);require(copy==a);auto moved=std::move(copy);require(copy.rows()==0&&copy.cols()==0);copy.reset(2,3);copy=std::move(moved);require(moved.empty()&&copy==a);swap(copy,moved);require(copy.empty()&&moved==a);moved.clear();require(moved.rows()==0&&moved.cols()==0);
 }
 for(int trial=0;trial<100;++trial){const auto r=rng(9),m=trial<8?std::array<std::size_t,8>{0,1,63,64,65,4095,4096,4097}[static_cast<std::size_t>(trial)]:rng(130),c=rng(10);Bytes a(r,std::vector<std::uint8_t>(m)),b(m,std::vector<std::uint8_t>(c));for(auto&row:a)for(auto&x:row)x=static_cast<std::uint8_t>(rng(2));for(auto&row:b)for(auto&x:row)x=static_cast<std::uint8_t>(rng(2));auto pa=packed(a,m),pb=packed(b,c);auto expected=product(a,b,c);compare(pa.multiply(pb),expected,c);require(pa*pb==pa.multiply(pb));pa*=pb;compare(pa,expected,c);}
 for(const std::size_t n:{1U,63U,64U,65U,129U}){auto p=BinaryMatrix(n,n);for(std::size_t i=0;i<n;++i)p.set(i,(i+1)%n);require(p.rank()==n&&p.determinant());auto inv=p.inverse();require(inv&&*inv*p==BinaryMatrix::identity(n));const auto k=std::numeric_limits<std::uint64_t>::max();auto q=p.pow(k);for(std::size_t i=0;i<n;++i)for(std::size_t j=0;j<n;++j)require(q(i,j)==(j==(i+k%n)%n));auto self=p;self*=self;require(self==p*p);}
}
void boundaries(){
 BinaryMatrix empty;require(empty.determinant());require(empty.inverse().has_value());require(empty.pow(UINT64_MAX)==empty);require(empty.solve({}).has_value());BinaryMatrix wide(0,SIZE_MAX),tall(SIZE_MAX,0);require(wide.rank()==0&&tall.rank()==0);require(wide.transpose()==tall);require((wide*tall)==empty);require(tall.row_words(SIZE_MAX-1).empty());rejected([&]{wide.solve({});});rejected([&]{empty.reset(SIZE_MAX,65);});require(empty.rows()==0&&empty.cols()==0);
 BinaryMatrix zero_columns(3,0);require(zero_columns.solve(std::vector<std::uint8_t>{0,0,0}).has_value());require(!zero_columns.solve(std::vector<std::uint8_t>{0,1,0}));
#ifndef SKIP_RUNTIME_CHECKS
 BinaryMatrix a(2,65),b(2,64);auto old=a;rejected([&]{a.set(2,0);});rejected([&]{a.test(0,65);});rejected([&]{a.flip(0,SIZE_MAX);});rejected([&]{a.row_words(2);});rejected([&]{a.swap_rows(0,2);});rejected([&]{a.xor_rows(0,1,66);});rejected([&]{a+=b;});rejected([&]{a*=b;});rejected([&]{a.determinant();});rejected([&]{a.inverse();});rejected([&]{a.pow(0);});rejected([&]{a.solve(std::vector<std::uint8_t>{0});});rejected([&]{a.solve(std::vector<std::uint8_t>{0,2});});require(a==old);rejected([&]{zero_columns.solve(std::vector<std::uint8_t>{2,0,0});});require(!zero_columns.solve(std::vector<std::uint8_t>{1,2,0}));BinaryMatrix z(0,1);rejected([&]{z.determinant();});rejected([&]{z.inverse();});rejected([&]{z.pow(0);});
#endif
}
int main(){small_exhaustive();random_cases();boundaries();std::cout<<"binary matrix independent checks="<<checks<<'\n';}
