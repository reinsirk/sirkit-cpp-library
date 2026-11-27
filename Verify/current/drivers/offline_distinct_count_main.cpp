// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{}, q{};
    verify_in >> n >> q;

    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    std::vector<DistinctCountQuery> queries(q);
    for (auto &[left, right] : queries)
    {
        verify_in >> left >> right;
    }
    for (auto count : offline_distinct_count(values, queries))
    {
        verify_out << count << '\n';
    }
}
