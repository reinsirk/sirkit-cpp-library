// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_MATRIX_TEST_SOURCE
#define SIRKIT_MATRIX_TEST_SOURCE "../../../Algorithm/rotate.cpp"
#endif
#include SIRKIT_MATRIX_TEST_SOURCE
#include SIRKIT_MATRIX_TEST_SOURCE
#include <bit>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <utility>
#include <cfenv>
enum class WideCode : std::uint64_t {};
struct WideBox { std::uint64_t bits; };
std::uint64_t checks=0;
template<class T> void test(std::size_t h,std::size_t w,std::mt19937_64& rng){
 std::vector<std::vector<T>> input(h,std::vector<T>(w));
 for(auto&row:input)for(auto&value:row)value=std::bit_cast<T>(rng());
 const auto original=input;
 for(int mode:{0,1,3,4,5,6}){
  std::feclearexcept(FE_ALL_EXCEPT);std::feraiseexcept(FE_DIVBYZERO);const int before=std::fetestexcept(FE_ALL_EXCEPT);
  auto b=mode==0?transpose(input):mode==1?rotate_90r(input):mode==3?rotate_90l(input):mode==4?rotate_quarter(input,5):mode==5?rotate_quarter(input,-5):transpose(transpose(transpose(input)));
  assert(before==std::fetestexcept(FE_ALL_EXCEPT));
  assert(b.size()==w);for(auto&row:b)assert(row.size()==h);
  for(std::size_t i=0;i<h;++i)for(std::size_t j=0;j<w;++j){const auto r=(mode==3||mode==5)?w-1-j:j;const auto c=(mode==1||mode==4)?h-1-i:i;assert(std::bit_cast<std::uint64_t>(b[r][c])==std::bit_cast<std::uint64_t>(input[i][j]));assert(std::bit_cast<std::uint64_t>(original[i][j])==std::bit_cast<std::uint64_t>(input[i][j]));++checks;}
 }
#ifndef SKIP_RUNTIME_CHECKS
 for(std::size_t bad:{std::size_t{0},h/2,h-1})if(h>1&&w>1){auto ragged=input;ragged[bad].pop_back();for(int mode:{0,1,3}){bool thrown=false;try{(void)(mode==0?transpose(ragged):mode==1?rotate_90r(ragged):rotate_90l(ragged));}catch(const std::invalid_argument&){thrown=true;}assert(thrown);++checks;}}
#endif
}
int main(){std::mt19937_64 rng(261029);for(auto[h,w]:{std::pair{31,129},{32,127},{32,128},{32,129},{33,127},{63,65},{64,64},{64,65},{65,63},{65,64},{65,65},{65,67},{127,129},{129,127},{257,255},{1025,1027},{32,16383},{32,16384},{33,15888},{64,8191},{64,8192},{65,8192},{8193,64}}){test<std::int64_t>(h,w,rng);test<std::uint64_t>(h,w,rng);test<double>(h,w,rng);test<WideCode>(h,w,rng);test<WideBox>(h,w,rng);}std::cout<<"PASS checks="<<checks<<'\n';}
