// SPDX-License-Identifier: CC0-1.0
#include "../Math/Gaussian_Elimination.cpp"
#include "../Math/Fraction.cpp"
#include <iostream>
#include <vector>

using namespace sirkit;
int main(){
    const Matrix<Fraction> a{{1,2},{3,5}};
    const auto reduced=GaussianElimination(a,true);
    std::cout<<reduced.rank()<<' '<<MatrixDeterminant(a)<<'\n'; // 2 -1/1
    const auto inverse=MatrixInverse(a);
    std::cout<<(inverse.has_value()&&a*(*inverse)==MatrixIdentity<Fraction>(2))<<'\n'; // 1
    const Matrix<Fraction> equation{{1,2,3}};
    const std::vector<Fraction> rhs{4};
    const auto solution=SolveLinearSystem(equation,rhs);
    std::cout<<solution->kernel.rows()<<' '<<(MatrixApply(equation,solution->particular)==rhs)<<'\n'; // 2 1
    for(const auto&x:solution->particular)std::cout<<x<<' '; // 4/1 0/1 0/1
    std::cout<<'\n';
    for(std::size_t i=0;i<solution->kernel.rows();++i){for(const auto&x:solution->kernel.row(i))std::cout<<x<<' ';std::cout<<'\n';}
    // -2/1 1/1 0/1 / -3/1 0/1 1/1
    std::cout<<MatrixRank(Matrix<Fraction>(0,3))<<' '<<MatrixDeterminant(Matrix<Fraction>(0,0))<<'\n'; // 0 1/1
}
