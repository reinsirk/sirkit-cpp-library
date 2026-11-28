// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tournament_Strong_Counts.cpp"

#include <iostream>

int main()
{
    const auto count = sirkit::tournament_strong_counts(3);
    std::cout << count[3] << '\n'; // 2
    const auto probabilities = sirkit::tournament_sink_size_probabilities(3);
    std::cout << probabilities[3] << '\n'; // 1/4 mod 998244353
}
