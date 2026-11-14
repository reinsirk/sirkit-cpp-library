// SPDX-License-Identifier: CC0-1.0
#include "../Math/Berlekamp_Massey.cpp"
#include "../Math/Fraction.cpp"
#include <cstdint>
#include <iostream>
#include <vector>

using namespace sirkit;
int main() {
    const auto fibonacci=BerlekampMassey(std::vector<std::uint32_t>{0,1,1,2,3,5,8,13});
    for(auto x:fibonacci)std::cout<<x<<' '; // 1 1
    std::cout<<'\n';
    const auto arithmetic=BerlekampMassey(std::vector<Fraction>{1,2,3,4,5,6});
    for(const auto&x:arithmetic)std::cout<<x<<' '; // 2/1 -1/1
    std::cout<<'\n';
}
