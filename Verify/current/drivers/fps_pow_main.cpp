// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <utility>
#include <vector>
int main() {
    int n{}; verify_in >> n;
    std::uint64_t exponent{}; verify_in >> exponent;
    std::vector<std::uint32_t> coefficients(n);
    for (auto& value : coefficients) verify_in >> value;
    const FormalPowerSeries f(std::move(coefficients));
    const auto answer = f.pow(exponent);
    for (std::size_t i = 0; i < answer.size(); ++i) verify_out << (i ? " " : "") << answer[i];
    verify_out << '\n';
}
