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
    verify_out<<MatrixDeterminant(a).get()<<'\n';
}
