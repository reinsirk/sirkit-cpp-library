// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Eytzinger_Search.cpp"
#include <cassert>
#include <vector>

int main()
{
    const sirkit::EytzingerSearch data(std::vector{3, 1, 3, 7});
    const auto [left, right] = data.equal_range(3);
    assert(left == 1 && right == 3);
    assert(data.lower_bound(8) == data.size());
}
