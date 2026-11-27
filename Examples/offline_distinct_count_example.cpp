// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Offline_Distinct_Count.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<std::int64_t> values{5, -2, 5, 3, -2};
    const std::vector<sirkit::DistinctCountQuery> queries{{0, 5}, {1, 3}, {3, 3}};
    assert((sirkit::offline_distinct_count(values, queries) == std::vector<std::size_t>{3, 2, 0}));
}
