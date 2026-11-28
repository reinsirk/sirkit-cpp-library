// SPDX-License-Identifier: CC0-1.0
#include "../Math/Relaxed_Convolution.cpp"
#include <iostream>

int main()
{
    sirkit::RelaxedConvolution convolution;
    std::cout << convolution.push(3, 5) << '\n'; // 3*5=15
    std::cout << convolution.push(4, 6) << '\n'; // 3*6+4*5=38
}
