// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Lazy_Segment_Tree.cpp"
#include <cstdint>
#include <iostream>
#include <vector>

using namespace sirkit;

// Sに当たる型。区間の「合計」と「要素数」を保持する。
struct Sum { std::int64_t value; int length; };
// Fに当たる型。「各要素にいくつ足すか」を表す。
using F = std::int64_t;

Sum op(Sum left, Sum right) {
    return {left.value + right.value, left.length + right.length};
}
Sum e() { return {0, 0}; } // 空区間。
Sum mapping(F add, Sum x) {
    return {x.value + add * x.length, x.length};
}
F composition(F newer, F older) { return newer + older; }
F id() { return 0; } // 何も足さない。

int main() {
    using Tree = lazy_segtree<Sum, op, e, F, mapping, composition, id>;
    std::vector<Sum> initial{{1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}};
    Tree tree(initial);

    tree.apply(1, 4, 3); // 添字1,2,3へ+3。列は[1,5,6,7,5]。
    std::cout << tree.prod(1, 4).value << '\n'; // 18 = 5+6+7。
    std::cout << tree.all_prod().value << '\n'; // 24。

    tree.set(2, Sum{10, 1}); // 添字2を10に置き換える。
    tree.apply(0, 2);       // 添字0だけに+2。列は[3,5,10,7,5]。
    std::cout << tree.get(0).value << '\n'; // 3。
}
