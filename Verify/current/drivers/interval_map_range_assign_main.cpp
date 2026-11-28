// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <limits>

int main()
{
    std::size_t n{}, q{};
    verify_in >> n >> q;
    sirkit::IntervalMap<int> values(n, std::numeric_limits<int>::max());
    for (std::size_t i = 0; i < q; ++i)
    {
        int type{};
        std::size_t left{};
        verify_in >> type >> left;
        if (type == 0)
        {
            std::size_t right{};
            int value{};
            verify_in >> right >> value;
            values.assign(left, right + 1, value);
        }
        else
        {
            verify_out << values.get(left) << '\n';
        }
    }
}
