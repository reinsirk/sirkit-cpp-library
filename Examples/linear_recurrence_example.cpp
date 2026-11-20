// SPDX-License-Identifier: CC0-1.0
#include "../Math/Linear_Recurrence.cpp"
#include "../Math/Fraction.cpp"
#include <cstdint>
#include <iostream>
#include <vector>

using namespace sirkit;
int main() {
    const std::vector<std::uint32_t> initial{0,1},coefficients{1,1};
    std::cout<<LinearRecurrenceTerm(initial,coefficients,10)<<'\n'; // 55
    for(auto x:LinearRecurrencePrefix(initial,coefficients,8))std::cout<<x<<' '; // 0 1 1 2 3 5 8 13
    std::cout<<'\n';
    for(auto x:LinearRecurrenceWeights(coefficients,10))std::cout<<x<<' '; // 34 55
    std::cout<<'\n';
    const std::vector<Fraction> a{1,2},c{2,-1};
    std::cout<<LinearRecurrenceTerm(a,c,5)<<'\n'; // 6/1
    const auto restored=BerlekampMassey(LinearRecurrencePrefix(a,c,8));
    std::cout<<(restored==c)<<'\n'; // 1
}
