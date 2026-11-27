// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Subset_Sum.cpp"
#include <cassert>

int main()
{
    const std::vector<std::size_t> values{2, 4, 9};
    const auto chosen = sirkit::subset_sum(values, 11);
    assert((chosen == std::vector<std::size_t>{0, 2}));
}
