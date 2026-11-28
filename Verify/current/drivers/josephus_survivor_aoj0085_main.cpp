// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    while (true)
    {
        std::uint64_t count{}, every{};
        verify_in >> count >> every;
        if (count == 0 && every == 0)
        {
            return 0;
        }
        verify_out << sirkit::josephus_survivor(count, every) + 1 << '\n';
    }
}
