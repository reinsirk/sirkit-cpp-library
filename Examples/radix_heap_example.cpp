// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Radix_Heap.cpp"

#include <cassert>

int main()
{
    sirkit::RadixHeap<unsigned, int> queue;
    queue.emplace(10, 0);
    queue.emplace(3, 1);
    const auto [distance, vertex] = queue.pop();
    assert(distance == 3 && vertex == 1);
    queue.emplace(distance + 2, 2);
    assert(queue.top().second == 2);
}
