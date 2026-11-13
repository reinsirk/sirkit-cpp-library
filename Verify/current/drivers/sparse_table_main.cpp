// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <limits>
#include <vector>
struct StaticRangeMinimum { int operator()(int a, int b) const { return std::min(a, b); } };
int main() {
    int n{}, q{}; verify_in >> n >> q;
    std::vector<int> a(n);
    for (auto& x : a) verify_in >> x;
    SparseTable<int, StaticRangeMinimum> table(a, std::numeric_limits<int>::max());
    while (q--) {
        int l{}, r{}; verify_in >> l >> r;
        verify_out << table.prod(l, r) << '\n';
    }
}
