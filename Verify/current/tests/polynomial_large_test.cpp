#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
#ifndef HEADER
#define HEADER "../../../Math/Polynomial.cpp"
#endif
#include HEADER
static std::uint32_t pow_ref(std::uint32_t a,std::size_t e){std::uint32_t r=1;for(;e;e/=2,a=static_cast<std::uint32_t>(std::uint64_t{a}*a%998244353))if(e&1)r=static_cast<std::uint32_t>(std::uint64_t{r}*a%998244353);return r;}
int main(){
 for(std::size_t n:{std::size_t{4097},std::size_t{131072}}){
  std::vector<std::uint32_t>points(n),f(n);for(std::size_t i=0;i<n;++i)points[i]=static_cast<std::uint32_t>(i*17+3);f.back()=1;
  ProductTree tree(points);const auto values=tree.evaluate(f);
  for(std::size_t i=0;i<n;++i)if(values[i]!=pow_ref(points[i],n-1))throw std::runtime_error("large Horner monomial mismatch");
  if(tree.interpolate(values).coefficients()!=f)throw std::runtime_error("large interpolation monomial mismatch");
  const auto shifted=TaylorShift(f,-7);
  const std::vector<std::uint32_t>probe{0,1,2,998244352,192837465};const auto shifted_values=MultipointEvaluate(shifted,probe);
  for(std::size_t i=0;i<probe.size();++i)if(shifted_values[i]!=pow_ref(static_cast<std::uint32_t>((std::uint64_t{probe[i]}+998244353-7)%998244353),n-1))throw std::runtime_error("large Taylor monomial mismatch");
 }
 std::cout<<"PASS nonpower and 131072-point monomial evaluation/interpolation/shift with independent modular powers\n";
}
