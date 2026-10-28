// SPDX-License-Identifier: CC0-1.0
// ABC421 F: https://atcoder.jp/contests/abc421/tasks/abc421_f
#include "../DataStructure/implicit_treap.cpp"
#include <iostream>

using IdSumTreap = indexed_implicit_treap<
    RangeAddSum::S, RangeAddSum::op, RangeAddSum::e, RangeAddSum::F,
    RangeAddSum::mapping, RangeAddSum::composition, RangeAddSum::id>;

int main() {
    int q;
    if (!(std::cin >> q)) return 0;
    IdSumTreap sequence;
    sequence.reserve(static_cast<std::size_t>(q) + 1);
    sequence.reserve_ids(static_cast<std::size_t>(q) + 1);
    sequence.insert_at_with_id(0, 0, {0, 1}); // 位置0に、ID=0、値=0を置く。
    for (int i = 1; i <= q; ++i) {
        int type, x;
        std::cin >> type >> x;
        if (type == 1) {
            sequence.insert_after(x, i, {i, 1}); // ID=xの直後へID=i、値=iを追加。
        } else {
            int y;
            std::cin >> y;
            // 端の2要素は残し、その間の合計を求めて削除する。
            std::cout << sequence.erase_between(x, y).sum << '\n';
        }
    }
}
