// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef HEADER
#define HEADER "../../../Math/Gaussian_Elimination.cpp"
#endif
#include HEADER
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
void check(bool x){if(!x)throw std::runtime_error("matrix ownership mismatch");}
struct alignas(64) Owned {
 static inline int live=0,budget=-1;
 std::uint32_t value;
 std::string storage;
 Owned()=delete;
 Owned(std::uint32_t x):value(x%7),storage(64,'v'){++live;}
 Owned(const Owned&x):value(x.value),storage(x.storage){if(budget==0)throw std::runtime_error("copy");if(budget>0)--budget;++live;}
 Owned(Owned&&x)noexcept:value(x.value),storage(std::move(x.storage)){++live;}
 Owned&operator=(const Owned&x){value=x.value;storage=x.storage;return *this;}
 Owned&operator=(Owned&&)=default;
 ~Owned(){--live;}
 Owned&operator+=(const Owned&x){value=(value+x.value)%7;return *this;}
 Owned&operator-=(const Owned&x){value=(value+7-x.value)%7;return *this;}
 Owned&operator*=(const Owned&x){value=value*x.value%7;return *this;}
 friend Owned operator+(Owned a,const Owned&b){return a+=b;}friend Owned operator-(Owned a,const Owned&b){return a-=b;}friend Owned operator*(Owned a,const Owned&b){return a*=b;}
 friend bool operator==(const Owned&a,const Owned&b){return a.value==b.value;}
 friend Owned operator/(const Owned&a,const Owned&b){std::uint32_t inverse=1;for(int i=0;i<5;++i)inverse=inverse*b.value%7;return Owned(a.value*inverse%7);}
};
struct NonField {
 std::uint32_t value=0;NonField()=default;NonField(std::uint32_t x):value(x%8){}
 NonField&operator+=(NonField x){value=(value+x.value)%8;return *this;}NonField&operator-=(NonField x){value=(value+8-x.value)%8;return *this;}NonField&operator*=(NonField x){value=value*x.value%8;return *this;}
 friend NonField operator+(NonField a,NonField b){return a+=b;}friend NonField operator-(NonField a,NonField b){return a-=b;}friend NonField operator*(NonField a,NonField b){return a*=b;}friend bool operator==(NonField,NonField)=default;
 friend NonField operator/(NonField a,NonField b){for(std::uint32_t i=1;i<8;++i)if(i*b.value%8==1)return a*NonField(i);return 0;}
};
int main(){
 for(int allowance=0;allowance<180;++allowance){
  {Matrix<Owned>a{{1,2},{3,4}},target{{6}};const auto initial=Owned::live;bool copied_correctly=true;Owned::budget=allowance;try{auto copy=a;copied_correctly=(copy==a);}catch(const std::runtime_error&){}Owned::budget=-1;check(copied_correctly&&Owned::live==initial);
   Owned::budget=allowance;bool threw=false;try{target=a;}catch(const std::runtime_error&){threw=true;}Owned::budget=-1;if(threw)check(target.rows()==1&&target(0,0).value==6);else check(target==a);
   auto before=Owned::live;Owned::budget=allowance;try{auto product=a*a;(void)product;}catch(const std::runtime_error&){}Owned::budget=-1;check(Owned::live==before);check(a(0,0).value==1&&a(1,1).value==4);
   Owned::budget=allowance;try{auto result=MatrixInverse(a);(void)result;}catch(const std::runtime_error&){}Owned::budget=-1;check(Owned::live==before);check(a(0,0).value==1&&a(1,1).value==4);
   std::vector<Owned>b{1,2};before=Owned::live;Owned::budget=allowance;try{auto solution=SolveLinearSystem(a,b);(void)solution;}catch(const std::runtime_error&){}Owned::budget=-1;check(Owned::live==before);
   check(reinterpret_cast<std::uintptr_t>(&a(0,0))%64==0);auto moved=std::move(a);check(a.rows()==0&&a.cols()==0&&a.empty());a.assign(1,1,Owned{1});check(moved.rows()==2&&a(0,0).value==1);
  }check(Owned::live==0);
 }
 bool rejected=false;try{(void)MatrixInverse(Matrix<NonField>{{2}});}catch(const std::domain_error&){rejected=true;}check(rejected);
 const auto maximum=std::numeric_limits<std::size_t>::max();Matrix<NonField>empty(0,maximum);auto transposed=empty.transposed();check(transposed.rows()==maximum&&transposed.cols()==0&&transposed.empty());check((transposed*Matrix<NonField>(0,0)).empty());check(MatrixRank(empty)==0);
 bool overflow=false;try{(void)SolveLinearSystem(empty,std::vector<NonField>{});}catch(const std::length_error&){overflow=true;}check(overflow);
 std::cout<<"PASS alignas64/nondefault/throwing-copy/deep copy/move/multiply/inverse/system, live0, nonunit and empty-dimension overflow\n";
}
