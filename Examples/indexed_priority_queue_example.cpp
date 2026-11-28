// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Indexed_Priority_Queue.cpp"

#include <cassert>

int main()
{
    sirkit::IndexedPriorityQueue<int> queue(5);
    queue.set(2, 7);
    queue.set(4, 4);
    queue.set(2, 1);
    const auto [id, priority] = queue.pop();
    assert(id == 2 && priority == 1 && !queue.contains(2));
    assert(queue.priority(4) == 4);
}
