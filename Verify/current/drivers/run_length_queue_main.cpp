// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    int queries{};
    verify_in >> queries;
    sirkit::RunLengthQueue<std::uint64_t> queue;
    for (int i = 0; i < queries; ++i)
    {
        int type{};
        std::uint64_t count{};
        verify_in >> type >> count;
        if (type == 1)
        {
            std::uint64_t value{};
            verify_in >> value;
            queue.push_back(value, count);
        }
        else
        {
            std::uint64_t sum = 0;
            queue.consume_front(count, [&](std::uint64_t value, std::uint64_t copies)
            {
                sum += value * copies;
            });
            verify_out << sum << '\n';
        }
    }
}
