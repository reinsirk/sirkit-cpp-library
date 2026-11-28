// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

int main()
{
    std::size_t n{}, q{};
    verify_in >> n >> q;
    std::vector<std::uint32_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const sirkit::SqrtTree tree(std::move(values), std::numeric_limits<std::uint32_t>::max(),
                                [](std::uint32_t a, std::uint32_t b)
                                {
                                    return std::min(a, b);
                                });
    for (std::size_t i = 0; i < q; ++i)
    {
        std::size_t left{}, right{};
        verify_in >> left >> right;
        verify_out << tree.prod(left, right) << '\n';
    }
}
