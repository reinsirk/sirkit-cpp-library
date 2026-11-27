// SPDX-License-Identifier: CC0-1.0
#include "../Math/Gaussian_Integer_GCD.cpp"
#include <cassert>

int main()
{
    const auto gcd = sirkit::gaussian_gcd({5, 1}, {1, 3});
    assert((gcd == sirkit::GaussianInteger{1, 1}));
}
