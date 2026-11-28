// SPDX-License-Identifier: CC0-1.0
#include "../String/String_Radix_Order.cpp"

#include <array>
#include <cassert>
#include <string_view>
#include <vector>

int main()
{
    const std::array<std::string_view, 5> words{"ab", "a", "ab", "ba", "b"};
    const auto order = sirkit::string_radix_order(words, "ba");
    assert((order == std::vector<std::size_t>{4, 3, 1, 0, 2}));
}
