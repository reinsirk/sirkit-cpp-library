// SPDX-License-Identifier: CC0-1.0
#include "../Math/Binary_Matrix.cpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

using namespace sirkit;
void print(const BinaryMatrix& a) {
    for(std::size_t i=0;i<a.rows();++i){for(std::size_t j=0;j<a.cols();++j)std::cout<<a.test(i,j);std::cout<<'\n';}
}
int main() {
    BinaryMatrix a(2,3);
    a.set(0,0).set(0,2).set(1,1).set(1,2);
    const auto transpose=a.transpose();
    print(transpose);
    const auto product=a*transpose;
    print(product);
    std::cout<<product.rank()<<' '<<product.determinant()<<'\n';
    if(const auto inverse=product.inverse())print(*inverse);
    print(product.pow(2));
    const auto solution=a.solve(std::vector<std::uint8_t>{1,0});
    if(solution){
        std::cout<<solution->kernel.rows()<<'\n';
        for(const auto value:solution->particular){std::cout<<static_cast<int>(value);}std::cout<<'\n';
        print(solution->kernel);
        for(const auto col:solution->pivot_columns){std::cout<<col<<' ';}std::cout<<'\n';
    }
    auto copy=a;
    copy.swap_rows(0,1);
    copy.xor_rows(1,0,1);
    copy.flip(0,0).set(0,2,false);
    print(copy);
    std::cout<<a.row_words(0)[0]<<' '<<a.rows()<<' '<<a.cols()<<' '<<a.empty()<<'\n';
    std::cout<<((a+a)==BinaryMatrix(2,3))<<' '<<((a^a)==BinaryMatrix(2,3))<<'\n';
    auto square=product;
    square*=square;
    std::cout<<(square==BinaryMatrix::identity(2))<<'\n';
    auto moved=std::move(copy);
    std::cout<<copy.empty()<<' '<<moved.rows()<<'\n';
    copy.reset(0,4);
    std::cout<<copy.rows()<<' '<<copy.cols()<<' '<<copy.rank()<<'\n';
    copy.clear();
}
