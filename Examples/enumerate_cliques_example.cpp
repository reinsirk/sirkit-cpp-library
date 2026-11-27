// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Enumerate_Cliques.cpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <span>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
    std::size_t count = 0;
    const auto visit = [&](std::span<const int>)
    {
        ++count;
    };
    sirkit::enumerate_cliques(3, edges, visit);
    assert(count == 5);
}
