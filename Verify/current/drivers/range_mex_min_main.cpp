// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n = 0, window = 0;
    verify_in >> n >> window;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    std::vector<sirkit::RangeMexQuery> queries(n - window + 1);
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        queries[i] = {i, i + window};
    }
    const auto mex = sirkit::range_mex(values, queries);
    verify_out << *std::min_element(mex.begin(), mex.end()) << '\n';
}
