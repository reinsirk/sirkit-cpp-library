// SPDX-License-Identifier: CC0-1.0
#include "../Math/Walsh_Hadamard.cpp"
#include <cassert>
#include <span>
#include <vector>

int main()
{
    const std::vector<long long> a{1, 2, 3, 4}, b{0, 1, 0, 0};
    const auto result = sirkit::bitwise_xor_convolution(a, b);
    assert((result == std::vector<long long>{2, 1, 4, 3}));

    auto data = a;
    sirkit::walsh_hadamard_transform(std::span(data));
    sirkit::inverse_walsh_hadamard_transform(std::span(data));
    assert(data == a);
}
