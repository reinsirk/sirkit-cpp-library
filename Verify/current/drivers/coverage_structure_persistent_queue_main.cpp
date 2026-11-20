// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>
int main() {
    int q{}; verify_in >> q;
    using Tree = PersistentSegmentTree<std::int64_t, std::plus<std::int64_t>>;
    Tree tree(static_cast<std::size_t>(q), {}, 0);
    tree.reserve(static_cast<std::size_t>(q) * (std::bit_width(static_cast<unsigned>(q - 1)) + 1));
    std::vector<Tree::Root> roots(q + 1, tree.empty_root());
    std::vector<std::size_t> left(q + 1), right(q + 1);
    for (int i = 1; i <= q; ++i) {
        int type{}, t{}; verify_in >> type >> t; ++t;
        left[i] = left[t]; right[i] = right[t]; roots[i] = roots[t];
        if (type == 0) { std::int64_t x{}; verify_in >> x; roots[i] = tree.set(roots[t], right[i]++, x); }
        else { verify_out << tree.get(roots[t], left[i]++) << '\n'; }
    }
}
