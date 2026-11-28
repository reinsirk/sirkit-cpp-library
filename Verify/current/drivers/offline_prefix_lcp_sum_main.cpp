// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

int main()
{
    std::size_t n = 0, q = 0;
    verify_in >> n;
    std::vector<std::string> strings(n);
    std::vector<std::string_view> initial;
    initial.reserve(n);
    for (auto &word : strings)
    {
        verify_in >> word;
        initial.push_back(word);
    }
    verify_in >> q;
    std::vector<sirkit::PrefixLcpOperation> queries;
    queries.reserve(q);
    for (std::size_t i = 0; i < q; ++i)
    {
        int type = 0;
        std::size_t index = 0;
        verify_in >> type >> index;
        if (type == 1)
        {
            std::string byte;
            verify_in >> byte;
            queries.emplace_back(sirkit::PrefixLcpAppend{index - 1, static_cast<std::uint8_t>(byte[0])});
        }
        else
        {
            queries.emplace_back(sirkit::PrefixLcpQuery{index - 1});
        }
    }
    for (const auto answer : sirkit::offline_prefix_lcp_sum(initial, queries))
    {
        verify_out << answer << '\n';
    }
}
