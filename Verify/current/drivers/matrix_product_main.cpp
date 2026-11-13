// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include "Verify/current/common/matrix_field.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
int main(){
    std::size_t n{},m{},k{};if(!(verify_in>>n>>m>>k))return 0;
    Matrix<VerifyMatrixField>a(n,m),b(m,k);
    for(auto& value:a.values()){std::uint32_t raw{};verify_in>>raw;value=VerifyMatrixField{raw};}
    for(auto& value:b.values()){std::uint32_t raw{};verify_in>>raw;value=VerifyMatrixField{raw};}
    const auto result=a*b;
    for(std::size_t i=0;i<result.rows();++i)for(std::size_t j=0;j<result.cols();++j)verify_out<<result(i,j).get()<<(j+1==result.cols()?'\n':' ');
}
