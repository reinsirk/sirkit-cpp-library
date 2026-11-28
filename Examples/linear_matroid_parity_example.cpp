// SPDX-License-Identifier: CC0-1.0
#include "../Math/Linear_Matroid_Parity.cpp"

#include <atcoder/modint>
#include <array>
#include <iostream>

int main()
{
    using Mint = atcoder::static_modint<998244353>;
    using Parity = sirkit::LinearMatroidParity<Mint>;
    Parity problem(4);
    const std::array first{Parity::Term{0, Mint{1}}}, second{Parity::Term{1, Mint{1}}};
    const std::array third{Parity::Term{2, Mint{1}}}, fourth{Parity::Term{3, Mint{1}}};
    problem.add_pair(first, second, Mint{37});
    problem.add_pair(third, fourth, Mint{91});
    std::cout << problem.sample_size() << '\n'; // 2
}
