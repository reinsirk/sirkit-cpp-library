// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    int queries{};
    verify_in >> queries;
    sirkit::OrderedMultiset<std::int64_t> values;
    for (int i = 0; i < queries; ++i)
    {
        int type{};
        std::int64_t x{};
        verify_in >> type >> x;
        if (type == 1)
        {
            values.insert(x);
            continue;
        }
        std::uint64_t k{};
        verify_in >> k;
        const std::int64_t *result = nullptr;
        if (type == 2)
        {
            const auto through = values.count_less_equal(x);
            if (k <= through)
            {
                result = values.kth(through - k);
            }
        }
        else
        {
            result = values.kth(values.count_less(x) + k - 1);
        }
        verify_out << (result ? *result : -1) << '\n';
    }
}
