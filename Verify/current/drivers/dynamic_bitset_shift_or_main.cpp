// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
int main() {
    int n{}; verify_in >> n;
    DynamicBitset reachable(10001);
    reachable.set(0);
    for (int i = 0; i < n; ++i) {
        std::size_t score{}; verify_in >> score;
        reachable.shift_or(score);
    }
    verify_out << reachable.count() << '\n';
}
