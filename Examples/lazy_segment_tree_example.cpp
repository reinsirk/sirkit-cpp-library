// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Lazy_Segment_Tree.cpp"
#include <cstdint>
#include <iostream>
#include <vector>

using namespace sirkit;

// S: 区間の合計と要素数。1要素は{値,1}、空区間は{0,0}。
struct Sum { std::int64_t value; int length; };
// F: 各要素への更新 x -> a*x+b。{1,b}は加算、{0,b}は代入。
struct Affine { std::int64_t a, b; };
Sum op(Sum left, Sum right) { return {left.value + right.value, left.length + right.length}; }
Sum e() { return {0, 0}; }
Sum mapping(Affine f, Sum sum) { return {f.a * sum.value + f.b * sum.length, sum.length}; }
Affine composition(Affine newer, Affine older) {
    // olderを先に、newerを後に適用する。
    return {newer.a * older.a, newer.a * older.b + newer.b};
}
Affine id() { return {1, 0}; } // x -> x。Sの単位元e()とは別。
bool at_most_five(Sum sum) { return sum.value <= 5; }

int main() {
    using Tree = lazy_segtree<Sum, op, e, Affine, mapping, composition, id>;
    Tree tree(std::vector<Sum>{{1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}});
    tree.apply(1, 4, Affine{1, 3}); // 加算: 1 5 6 7 5
    tree.apply(2, 5, Affine{0, 2}); // 代入: 1 5 2 2 2
    tree.apply(0, Affine{2, 0}); // 1点へ作用: 2 5 2 2 2
    tree.set(1, Sum{1, 1});
    std::cout << tree.prod(0, 5).value << '\n'; // 9
    std::cout << tree.get(2).value << '\n'; // 2
    std::cout << tree.max_right<at_most_five>(0) << '\n'; // 3
    std::cout << tree.min_left(5, [](Sum sum) { return sum.value <= 4; }) << '\n'; // 3
    tree.apply(0, tree.size(), Affine{1, 1});
    for (auto sum : tree.values()) std::cout << sum.value << ' ';
    std::cout << '\n'; // 3 2 3 3 3
}
