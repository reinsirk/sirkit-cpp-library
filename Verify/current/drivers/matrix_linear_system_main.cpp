// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include "Verify/current/common/matrix_field.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
int main(){
    std::size_t n{},m{};if(!(verify_in>>n>>m))return 0;
    Matrix<VerifyMatrixField>a(n,m);
    for(auto& value:a.values()){std::uint32_t raw{};verify_in>>raw;value=VerifyMatrixField{raw};}
    std::vector<VerifyMatrixField>b(n);
    for(auto& value:b){std::uint32_t raw{};verify_in>>raw;value=VerifyMatrixField{raw};}
    const auto solution=SolveLinearSystem(a,b);
    if(!solution){verify_out<<-1<<'\n';return 0;}
    verify_out<<solution->kernel.rows()<<'\n';
    for(std::size_t j=0;j<m;++j)verify_out<<solution->particular[j].get()<<(j+1==m?'\n':' ');
    const auto& result=solution->kernel;
    for(std::size_t i=0;i<result.rows();++i)for(std::size_t j=0;j<result.cols();++j)verify_out<<result(i,j).get()<<(j+1==result.cols()?'\n':' ');
}
