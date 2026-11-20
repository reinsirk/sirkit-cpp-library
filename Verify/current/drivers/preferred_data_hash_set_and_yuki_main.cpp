// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
int main() {
    int q{}; verify_in >> q;
    HashSet<std::int64_t> values;
    std::array<int, 30> counts{};
    while (q--) {
        int type{}; verify_in >> type;
        if (type == 3) {
            if (values.empty()) { verify_out << -1 << '\n'; continue; }
            std::int64_t answer = 0;
            for (int bit = 0; bit < 30; ++bit)
                if (static_cast<std::size_t>(counts[bit]) == values.size()) answer |= std::int64_t{1} << bit;
            verify_out << answer << '\n';
            continue;
        }
        std::int64_t x{}; verify_in >> x;
        int delta = 0;
        if (type == 1) { if (values.insert_checked(x).second) delta = 1; }
        else {
            const auto found = values.find(x);
            if (found != values.end()) { values.erase(found, false); delta = -1; }
        }
        if (delta) for (int bit = 0; bit < 30; ++bit) counts[bit] += delta * static_cast<int>((x >> bit) & 1);
    }
}
