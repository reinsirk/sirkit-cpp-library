// SPDX-License-Identifier: CC0-1.0
#include "../String/Suffix_Tree.cpp"
#include <cassert>

int main()
{
    const sirkit::SuffixTree tree("bananabanana");
    assert(!tree.children(0).empty());
    const auto repeat = tree.longest_nonoverlapping_repeat();
    assert(repeat.first_start == 0);
    assert(repeat.second_start == 6);
    assert(repeat.length == 6);
}
