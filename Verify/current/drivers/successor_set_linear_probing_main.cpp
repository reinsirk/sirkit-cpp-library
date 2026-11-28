// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    constexpr int kSize = 1 << 20;
    sirkit::SuccessorSet available(kSize);
    std::vector<std::int64_t> values(kSize, -1);
    int q{};
    verify_in >> q;
    for (int i = 0; i < q; ++i)
    {
        int type{};
        std::int64_t value{};
        verify_in >> type >> value;
        const auto position = static_cast<int>(value % kSize);
        if (type == 1)
        {
            values[*available.take_cyclic(position)] = value;
        }
        else
        {
            verify_out << values[position] << '\n';
        }
    }
}
