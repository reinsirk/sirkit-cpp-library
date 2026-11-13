// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

using VerifyIdSumTreap = indexed_implicit_treap<
    RangeAddSum::S, RangeAddSum::op, RangeAddSum::e, RangeAddSum::F,
    RangeAddSum::mapping, RangeAddSum::composition, RangeAddSum::id>;

int main() {
    int q;
    if (!(verify_in >> q)) return 0;
    VerifyIdSumTreap sequence;
    sequence.reserve(static_cast<std::size_t>(q) + 1);
    sequence.reserve_ids(static_cast<std::size_t>(q) + 1);
    sequence.insert_at_with_id(0, 0, {0, 1});
    for (int i = 1; i <= q; ++i) {
        int type, x;
        verify_in >> type >> x;
        if (type == 1) sequence.insert_after(x, i, {i, 1});
        else {
            int y;
            verify_in >> y;
            verify_out << sequence.erase_between(x, y).sum << '\n';
        }
    }
}
