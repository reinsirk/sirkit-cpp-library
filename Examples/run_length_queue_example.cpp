// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Run_Length_Queue.cpp"

#include <cassert>
#include <cstdint>

int main()
{
    sirkit::RunLengthQueue<int> queue;
    queue.push_back(3, 2);
    queue.push_back(5, 4);
    std::uint64_t sum = 0;
    queue.consume_front(3, [&](int value, std::uint64_t count)
    {
        sum += static_cast<std::uint64_t>(value) * count;
    });
    assert(sum == 11 && queue.front_count() == 3);
}
