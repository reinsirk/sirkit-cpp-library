// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Persistent_Queue.cpp"

#include <cassert>

int main()
{
    sirkit::PersistentQueue<int> queue;
    const auto one = queue.push(queue.empty_version(), 5);
    const auto two = queue.push(one, 8);
    const auto branch = queue.push(one, 13);
    const auto rest = queue.pop(two);
    assert(queue.front(rest) == 8);
    assert(queue.front(branch) == 5);
    assert(queue.front(queue.pop(branch)) == 13);
}
