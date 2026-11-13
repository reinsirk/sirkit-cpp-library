// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
int main() {
    std::size_t n{}, q{}; verify_in >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto& value : values) verify_in >> value;
    SegmentTreeBeats tree(values);
    while (q--) {
        unsigned type{}; std::size_t l{}, r{}; verify_in >> type >> l >> r;
        if (type == 3) verify_out << tree.sum(l, r) << '\n';
        else {
            std::int64_t x{}; verify_in >> x;
            if (type == 0) tree.chmin(l, r, x);
            else if (type == 1) tree.chmax(l, r, x);
            else tree.add(l, r, x);
        }
    }
}
