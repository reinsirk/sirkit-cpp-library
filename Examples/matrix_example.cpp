// SPDX-License-Identifier: CC0-1.0
#include "../Math/Matrix.cpp"
#include <cstdint>
#include <iostream>
#include <vector>
int main(){
    const Matrix<std::int64_t> a{{1,2},{3,4}},b{{5,6},{7,8}};
    const auto product=a*b;
    for(std::size_t i=0;i<product.rows();++i){for(auto x:product.row(i))std::cout<<x<<' ';std::cout<<'\n';}
    // 19 22 / 43 50
    const auto transpose=a.transposed();
    std::cout<<transpose(0,1)<<' '<<(a+Matrix<std::int64_t>(2,2)-a==Matrix<std::int64_t>(2,2))<<'\n'; // 3 1
    const auto power=MatrixPower(Matrix<std::int64_t>{{1,1},{1,0}},10);
    std::cout<<power(0,0)<<' '<<power(0,1)<<'\n'; // 89 55
    const auto applied=MatrixApply(a,std::vector<std::int64_t>{1,2});
    std::cout<<applied[0]<<' '<<applied[1]<<'\n'; // 5 11
    auto owned=Matrix<std::int64_t>(1,3,std::vector<std::int64_t>{2,3,5});
    auto moved=std::move(owned);owned.assign(1,1,7);
    std::cout<<moved(0,2)<<' '<<owned(0,0)<<'\n'; // 5 7
}
