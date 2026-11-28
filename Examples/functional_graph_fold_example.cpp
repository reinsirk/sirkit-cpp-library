// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Functional_Graph_Fold.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <utility>

int main()
{
    const std::array next{1, 2, 1};
    const std::array<std::uint64_t, 3> label{10, 2, 5};
    const sirkit::FunctionalGraphFold<std::uint64_t> walk(next, label, 0);
    const auto [finish, sum] = walk.walk(0, 5);
    assert(finish == 1 && sum == 24);
}
