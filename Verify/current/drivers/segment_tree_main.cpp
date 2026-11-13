// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>
std::int64_t range_sum_op(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t range_sum_identity() { return 0; }
int main() {
    int n{}, q{}; verify_in >> n >> q;
    std::vector<std::int64_t> a(n);
    for (auto& x : a) verify_in >> x;
    segtree<std::int64_t, range_sum_op, range_sum_identity> tree(a);
    while (q--) {
        int type{}, p{}; std::int64_t x{}; verify_in >> type >> p >> x;
        if (type == 0) tree.set(p, tree.get(p) + x);
        else verify_out << tree.prod(p, static_cast<int>(x)) << '\n';
    }
}
