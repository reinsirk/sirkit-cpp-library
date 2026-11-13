// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include "Verify/current/common/matrix_field.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
int main(){
    std::size_t n{};if(!(verify_in>>n))return 0;
    Matrix<VerifyMatrixField>a(n,n);
    for(auto& value:a.values()){std::uint32_t raw{};verify_in>>raw;value=VerifyMatrixField{raw};}
    const auto inverse=MatrixInverse(a);
    if(!inverse){verify_out<<-1<<'\n';return 0;}
    const auto& result=*inverse;
    for(std::size_t i=0;i<result.rows();++i)for(std::size_t j=0;j<result.cols();++j)verify_out<<result(i,j).get()<<(j+1==result.cols()?'\n':' ');
}
