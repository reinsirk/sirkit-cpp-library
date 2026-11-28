// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Maximum_Allowed_Pair.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::int64_t> left{2, 1}, right{10, 30, 20};
    const std::vector<std::pair<std::size_t, std::size_t>> forbidden{{0, 1}, {1, 0}, {1, 2}};
    const auto result = sirkit::maximum_allowed_pair(left, right, forbidden);
    assert(result && result->sum == 31 && result->left == 1 && result->right == 1);
}
