// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Ring_Deque.cpp"

#include <cassert>

int main()
{
    sirkit::RingDeque<int> deque;
    deque.push_back(4);
    deque.push_front(1);
    deque.emplace_back(9);
    assert(deque[1] == 4);
    deque.pop_front();
    assert(deque.front() == 4);
    assert(deque.back() == 9);
}
