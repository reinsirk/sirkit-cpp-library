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
    NTT<> engine;
    const auto result = engine.convolution_normalized(std::move(a), std::move(b));
    for (auto value : result) verify_out << value << ' ';
    verify_out << '\n';
}
