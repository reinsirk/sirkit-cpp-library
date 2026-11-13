// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <utility>
#include <vector>
int main() {
    int n{}; verify_in >> n;
    std::vector<std::uint32_t> coefficients(n);
    for (auto& value : coefficients) verify_in >> value;
    const FormalPowerSeries f(std::move(coefficients));
    const auto answer = f.sqrt();
    if (!answer) { verify_out << "-1\n"; return 0; }
    for (std::size_t i = 0; i < answer->size(); ++i) verify_out << (i ? " " : "") << (*answer)[i];
    verify_out << '\n';
}
