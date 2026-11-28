// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Parallel_Binary_Search.cpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <vector>

int main()
{
    const std::array events{2, 4, 1};
    const std::array targets{0, 4, 8};
    int sum = 0;
    const auto steps = sirkit::parallel_binary_search(
        events.size(), targets.size(),
        [&] { sum = 0; },
        [&](std::size_t i) { sum += events[i]; },
        [&](std::size_t q) { return sum >= targets[q]; });
    assert((steps == std::vector<std::size_t>{0, 2, 4}));
}
