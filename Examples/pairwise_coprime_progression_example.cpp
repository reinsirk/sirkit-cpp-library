// SPDX-License-Identifier: CC0-1.0
#include "../Math/Pairwise_Coprime_Progression.cpp"

#include <iostream>

int main()
{
    const auto first = sirkit::pairwise_coprime_progression_start(3, 6);
    std::cout << sirkit::max_pairwise_coprime_progression_length(6) << ' ' << first->to_string() << '\n'; // 9 7
}
