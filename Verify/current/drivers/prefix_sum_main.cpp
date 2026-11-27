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
    const PrefixSum<std::int64_t> sums(values);
    for (std::size_t i = 0; i < q; ++i)
    {
        std::size_t left{}, right{};
        verify_in >> left >> right;
        verify_out << sums.sum(left, right) << '\n';
    }
}
