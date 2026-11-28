// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <limits>
#include <span>
#include <vector>

int main()
{
    constexpr int kIdentity = std::numeric_limits<int>::max();
    const auto minimum = [](int lhs, int rhs) noexcept
    {
        return std::min(lhs, rhs);
    };
    for (;;)
    {
        std::size_t h, w, q;
        verify_in >> h >> w >> q;
        if (h == 0)
        {
            break;
        }
        std::vector<int> grid(h * w);
        for (auto &value : grid)
        {
            verify_in >> value;
        }
        const sirkit::SegmentTree2D tree(h, w, std::span<const int>(grid), kIdentity, minimum);
        while (q-- != 0)
        {
            std::size_t top, left, bottom, right;
            verify_in >> top >> left >> bottom >> right;
            verify_out << tree.prod(top, left, bottom + 1, right + 1) << '\n';
        }
    }
}
