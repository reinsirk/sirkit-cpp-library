// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Double_Ended_Priority_Queue.cpp"
#include <cassert>

int main()
{
    sirkit::DoubleEndedPriorityQueue<int> queue({3, 5, 1});
    assert(queue.pop_min() == 1);
    assert(queue.pop_max() == 5);
}
