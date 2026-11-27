// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    std::vector<RangeMajorityOperation<int>> operations;
    operations.reserve(q);
    for (int i = 0; i < q; ++i)
    {
        int type = 0;
        verify_in >> type;
        if (type == 0)
        {
            std::size_t position = 0;
            int value = 0;
            verify_in >> position >> value;
            operations.emplace_back(RangeMajoritySet<int>{position, value});
        }
        else
        {
            std::size_t left = 0, right = 0;
            verify_in >> left >> right;
            operations.emplace_back(RangeMajorityQuery{left, right});
        }
    }
    for (const auto &answer : range_majority_queries(values, operations))
    {
        verify_out << answer.value_or(-1) << '\n';
    }
}
