// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>
int main() {
    int n{}, q{}; verify_in >> n >> q;
    std::vector<std::int64_t> a(n);
    for (auto& x : a) verify_in >> x;
    fenwick_tree<std::int64_t> tree(a);
    while (q--) {
        int type{}, l{}; std::int64_t r{}; verify_in >> type >> l >> r;
        if (type == 0) tree.add(l, r);
        else verify_out << tree.sum(l, static_cast<int>(r)) << '\n';
    }
}
