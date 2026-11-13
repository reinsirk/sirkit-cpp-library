// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
int main() {
    std::uint64_t value{}, exponent{}; verify_in >> value >> exponent;
    verify_out << ModPow(value, exponent, 1'000'000'007) << '\n';
}
