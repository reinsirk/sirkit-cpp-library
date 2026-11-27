// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n, q;
    verify_in >> n >> q;
    std::vector<std::uint64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    std::vector<RangeXorQuery> queries(q);
    for (auto &query : queries)
    {
        verify_in >> query.left >> query.right >> query.x;
        --query.left;
    }
    for (const auto &answer : range_xor_queries(values, queries))
    {
        verify_out << (answer.can_make ? "Yes\n" : "No\n");
    }
}
