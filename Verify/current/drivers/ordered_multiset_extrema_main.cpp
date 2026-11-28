// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    int queries{};
    verify_in >> queries;
    sirkit::OrderedMultiset<int> values;
    for (int i = 0; i < queries; ++i)
    {
        int type{};
        verify_in >> type;
        if (type == 3)
        {
            verify_out << *values.kth(values.size() - 1) - *values.kth(0) << '\n';
            continue;
        }
        int x{};
        verify_in >> x;
        if (type == 1)
        {
            values.insert(x);
        }
        else
        {
            std::uint64_t amount{};
            verify_in >> amount;
            values.erase(x, amount);
        }
    }
}
