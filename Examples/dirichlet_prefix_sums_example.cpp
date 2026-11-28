// SPDX-License-Identifier: CC0-1.0
#include "../Math/Dirichlet_Prefix_Sums.cpp"
#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    const sirkit::DirichletPrefixSums<std::int64_t> layout(10);
    std::vector<std::int64_t> ones(layout.size());
    for (std::uint64_t i = 0; i < layout.size(); ++i)
    {
        ones[i] = static_cast<std::int64_t>(layout.quotient(i));
    }
    std::cout << layout.convolution(ones, ones).back() << '\n'; // 27
    std::cout << layout.inverse(ones, 1).back() << '\n';        // -1
}
