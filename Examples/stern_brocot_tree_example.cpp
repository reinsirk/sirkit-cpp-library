// SPDX-License-Identifier: CC0-1.0
#include "../Math/Stern_Brocot_Tree.cpp"
#include <cassert>

int main()
{
    using Tree = sirkit::SternBrocotTree;
    const auto path = Tree::encode({5, 3});
    assert((Tree::decode(path) == Tree::Fraction{5, 3}));
    assert((Tree::ancestor({5, 3}, 1) == Tree::Fraction{2, 1}));
    const auto [lower, upper] = Tree::range({5, 3});
    assert((lower == Tree::Fraction{3, 2} && upper == Tree::Fraction{2, 1}));
    const auto [round_down, round_up] = Tree::approximate({5, 3}, 3, 3);
    assert((round_down == Tree::Fraction{3, 2} && round_up == Tree::Fraction{2, 1}));
}
