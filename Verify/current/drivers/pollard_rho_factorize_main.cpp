// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
int main() {
    int q{}; verify_in >> q;
    PollardRho64 engine;
    while (q--) {
        std::uint64_t n{}; verify_in >> n;
        const auto factors = engine.prime_factors(n);
        verify_out << factors.size();
        for (auto factor : factors) verify_out << ' ' << factor;
        verify_out << '\n';
    }
}
