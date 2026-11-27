// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{}, q{};
    verify_in >> n >> q;
    std::vector<std::int32_t> sequence(n);
    for (auto &value : sequence)
    {
        verify_in >> value;
    }
    const sirkit::StaticRangeLIS lis(sequence);
    while (q-- != 0)
    {
        std::size_t left{}, right{};
        verify_in >> left >> right;
        verify_out << lis.length(left, right) << '\n';
    }
}
