// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <iostream>
#include <vector>
#include "../Math/Polynomial.cpp"

using namespace sirkit;
int main(){
    const std::vector<int> points{0,1,2};
    const FormalPowerSeries f{1,2,3};
    ProductTree tree(points);
    const auto values=tree.evaluate(f);
    for(auto value:values)std::cout<<value<<' ';
    std::cout<<'\n';
    const auto interpolated=tree.interpolate(values);
    for(auto value:interpolated.coefficients())std::cout<<value<<' ';
    std::cout<<'\n';
    const auto shifted=TaylorShift(f,1);
    for(auto value:shifted.coefficients())std::cout<<value<<' ';
    std::cout<<'\n';
    std::cout<<tree.size()<<' '<<tree.empty()<<' '<<tree.points()[1]<<' '<<tree.polynomial().get(3)<<'\n';
    std::cout<<MultipointEvaluate(std::vector<int>{1,2,3},points)[1]<<' '
             <<PolynomialInterpolate(points,values).get(2)<<' '
             <<TaylorShift(std::vector<int>{1,2,3},-1).get(0)<<'\n';
    auto copy=tree;
    tree.assign(std::vector<std::int64_t>{-1,4});
    swap(tree,copy);
    std::cout<<tree.size()<<' '<<copy.size()<<' '<<ProductTree::max_points()<<'\n';
}
