// SPDX-License-Identifier: CC0-1.0
#include "../Math/Polynomial_Prefix_Sum.cpp"
#include <cassert>

int main()
{
    const auto sum = sirkit::PolynomialPrefixSum(sirkit::FormalPowerSeries{0, 1});
    assert((sum == sirkit::FormalPowerSeries{0, 499122176, 499122177}));
}
