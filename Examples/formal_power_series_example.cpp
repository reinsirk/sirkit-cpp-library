// SPDX-License-Identifier: CC0-1.0
#include "../Math/Formal_Power_Series.cpp"
#include <iostream>

using namespace sirkit;
int main() {
    const FormalPowerSeries f{1, 2, 1};
    const auto inverse = f.inverse(6);
    for (auto value : inverse.coefficients()) std::cout << value << ' ';
    std::cout << '\n';
    const auto restored = f.log(6).exp(6);
    std::cout << (restored == f.prefix(6)) << '\n';
    const auto power = f.pow(3, 7);
    const auto root = f.sqrt(4);
    std::cout << power.size() << ' ' << root.has_value() << '\n';
    const auto [quotient, remainder] = (f * f).divmod(f);
    std::cout << (quotient == f) << ' ' << remainder.empty() << '\n';
    std::cout << (f.derivative().integral() == FormalPowerSeries{0, 2, 1}) << '\n';
    FormalPowerSeries::clear_cache();
}
