// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
std::uint8_t any_op(std::uint8_t x, std::uint8_t y) { return static_cast<std::uint8_t>(x | y); }
std::uint8_t any_identity() { return 0; }
int main() {
    int n{}, q{}; std::string initial; verify_in >> n >> q >> initial;
    auto tree = [&] {
        if (std::count(initial.begin(), initial.end(), '1') <= n / 64) {
            segtree<std::uint8_t, any_op, any_identity> result(n);
            for (int i = 0; i < n; ++i) if (initial[i] == '1') result.set(i, 1);
            return result;
        }
        std::vector<std::uint8_t> values(n);
        for (int i = 0; i < n; ++i) values[i] = static_cast<std::uint8_t>(initial[i] - '0');
        return segtree<std::uint8_t, any_op, any_identity>(values);
    }();
    while (q--) {
        int type{}, k{}; verify_in >> type >> k;
        if (type < 2) {
            const char value = type == 0 ? '1' : '0';
            if (initial[k] != value) { tree.set(k, static_cast<std::uint8_t>(value - '0')); initial[k] = value; }
        }
        else if (type == 2) verify_out << static_cast<int>(tree.get(k)) << '\n';
        else if (type == 3) {
            const int result = tree.max_right(k, [](std::uint8_t any) { return any == 0; });
            verify_out << (result == n ? -1 : result) << '\n';
        } else {
            const int result = tree.min_left(k + 1, [](std::uint8_t any) { return any == 0; });
            verify_out << (result ? result - 1 : -1) << '\n';
        }
    }
}
