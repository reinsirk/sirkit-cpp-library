// SPDX-License-Identifier: CC0-1.0
#include "../String/Offline_Prefix_Lcp_Sum.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <string_view>
#include <vector>

int main()
{
    const std::array<std::string_view, 3> words{"a", "ab", "b"};
    const std::array<sirkit::PrefixLcpOperation, 3> operations{
        sirkit::PrefixLcpQuery{0},
        sirkit::PrefixLcpAppend{0, 'b'},
        sirkit::PrefixLcpQuery{1},
    };
    assert((sirkit::offline_prefix_lcp_sum(words, operations) == std::vector<std::uint64_t>{2, 4}));
}
