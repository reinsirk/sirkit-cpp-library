// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
int main() {
    std::size_t tests{}; verify_in >> tests;
    while (tests--) {
        std::uint64_t n{}, s{}, k{}; verify_in >> n >> s >> k;
        const auto answer = SolveLinearCongruence(k, n - s, n);
        if (answer) verify_out << answer->remainder << '\n';
        else verify_out << -1 << '\n';
    }
}
