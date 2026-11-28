// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    std::size_t n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const sirkit::StaticRangeMinimum<int> index(std::move(values));
    for (std::size_t i = 0; i < q; ++i)
    {
        std::size_t left = 0, right = 0;
        verify_in >> left >> right;
        verify_out << index.min(left, right) << '\n';
    }
}
