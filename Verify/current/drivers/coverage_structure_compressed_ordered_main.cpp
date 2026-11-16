// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
int main() {
    int n{}, q{}; verify_in >> n >> q;
    CompressedBinaryTrie<std::uint32_t, 29> values;
    values.reserve(static_cast<std::size_t>(n) + static_cast<std::size_t>(q));
    while (n--) { std::uint32_t x{}; verify_in >> x; values.insert_unique(x); }
    while (q--) {
        int type{}; std::uint32_t x{}; verify_in >> type >> x;
        if (type == 0) values.insert_unique(x);
        else if (type == 1) values.del(x);
        else if (type == 2) verify_out << (x <= values.size() ? static_cast<int>(values.get_kth(x).first) : -1) << '\n';
        else if (type == 3) verify_out << static_cast<int>(values.count_less(x + 1)) << '\n';
        else if (type == 4) {
            const auto count = values.count_less(x + 1);
            verify_out << (count ? static_cast<int>(values.get_kth(count).first) : -1) << '\n';
        } else {
            const auto count = values.count_less(x);
            verify_out << (count < values.size() ? static_cast<int>(values.get_kth(count + 1).first) : -1) << '\n';
        }
    }
}
