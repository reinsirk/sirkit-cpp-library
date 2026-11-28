// SPDX-License-Identifier: CC0-1.0
#include "../Math/Labeled_Tree_Matching_Counts.cpp"

#include <iostream>

int main()
{
    const auto roots = sirkit::labeled_tree_exposable_root_counts(4);
    const auto perfect = sirkit::labeled_tree_perfect_matching_counts(4);
    std::cout << roots[4] << '\n'
              << perfect[4] << '\n'; // 3, 12
}
