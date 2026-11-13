// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
int main() {
    std::size_t tests{}; verify_in >> tests;
    while (tests--) {
        std::uint64_t x{}, y{}, p{}, q{}; verify_in >> x >> y >> p >> q;
        std::optional<std::uint64_t> answer;
        for (std::uint64_t first = x; first < x + y; ++first)
            for (std::uint64_t second = p; second < p + q; ++second) {
                const auto joined = ChineseRemainder({first, 2 * (x + y)}, {second, p + q});
                if (joined && (!answer || joined->remainder < *answer)) answer = joined->remainder;
            }
        if (answer) verify_out << *answer << '\n';
        else verify_out << "infinity\n";
    }
}
