// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>

int main()
{
    std::size_t n = 0;
    int q = 0;
    verify_in >> n >> q;
    auto data = [&]
    {
        std::string initial;
        initial.reserve(n);
        verify_in >> initial;
        return HierarchicalBitset(initial);
    }();
    while (q-- > 0)
    {
        int type = 0;
        std::size_t position = 0;
        verify_in >> type >> position;
        if (type == 0)
        {
            data.insert(position);
        }
        else if (type == 1)
        {
            data.erase(position);
        }
        else if (type == 2)
        {
            verify_out << static_cast<int>(data.contains(position)) << '\n';
        }
        else
        {
            const auto value = type == 3 ? data.next(position) : data.prev(position);
            verify_out << (value ? static_cast<int>(*value) : -1) << '\n';
        }
    }
}
