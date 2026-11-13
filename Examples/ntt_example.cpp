// SPDX-License-Identifier: CC0-1.0
#include "../Math/NTT.cpp"
#include <iostream>
int main() {
    const std::vector<std::int64_t> a{1, -2, 3}, b{4, 5};
    NTT<> engine;
    for (auto coefficient : engine.convolution(a, b)) std::cout << coefficient << ' ';
    std::cout << '\n';
    const auto squared = engine.square(a);
    std::cout << squared.size() << '\n';
    std::vector<std::uint32_t> values{1, 2, 3, 4};
    engine.transform(values);
    engine.transform(values, true);
    for (auto coefficient : values) std::cout << coefficient << ' ';
    std::cout << '\n';
    const auto moved = engine.convolution_normalized(std::move(values), {1, 1});
    const auto simple = ConvolutionMod(a, b);
    std::cout << moved.size() << ' ' << simple.size() << '\n';
    engine.clear_cache();
}
