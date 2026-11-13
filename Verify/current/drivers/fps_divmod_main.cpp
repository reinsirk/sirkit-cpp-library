// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <utility>
#include <vector>
int main() {
    int n{}, m{}; verify_in >> n >> m;
    std::vector<std::uint32_t> a(n), b(m);
    for (auto& value : a) verify_in >> value;
    for (auto& value : b) verify_in >> value;
    const auto [quotient, remainder] = FormalPowerSeries(std::move(a)).divmod(FormalPowerSeries(std::move(b)));
    verify_out << quotient.size() << ' ' << remainder.size() << '\n';
    for (std::size_t i = 0; i < quotient.size(); ++i) verify_out << (i ? " " : "") << quotient[i];
    verify_out << '\n';
    for (std::size_t i = 0; i < remainder.size(); ++i) verify_out << (i ? " " : "") << remainder[i];
    verify_out << '\n';
}
