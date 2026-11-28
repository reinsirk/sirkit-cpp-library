// SPDX-License-Identifier: CC0-1.0
#include "../Math/Divisibility_Chains.cpp"

#include <iostream>

int main()
{
    const auto ending = sirkit::divisibility_chain_end_counts(3, 4);
    std::cout << ending[4] << '\n';                                   // 6: (1,1,4), (1,2,4), (1,4,4), (2,2,4), (2,4,4), (4,4,4)
    std::cout << sirkit::divisibility_chain_product_count(4) << '\n'; // 2
}
