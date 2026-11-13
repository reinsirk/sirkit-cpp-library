// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
int main() {
    std::size_t tests{}; verify_in >> tests;
    while (tests--) {
        std::uint64_t value{}, prime{}; verify_in >> value >> prime;
        const auto root = ModSqrt(value, prime);
        if (root) verify_out << *root << '\n';
        else verify_out << -1 << '\n';
    }
}
