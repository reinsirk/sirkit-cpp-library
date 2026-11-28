// SPDX-License-Identifier: CC0-1.0
#include "../Math/Permutation_Comparison_Count.cpp"

#include <iostream>

int main()
{
    std::cout << sirkit::count_permutations_by_comparisons("<>") << '\n'; // 2
    std::cout << sirkit::count_permutations_by_comparisons("<?") << '\n'; // 3
}
