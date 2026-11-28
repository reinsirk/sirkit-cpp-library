// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Meldable_Heap.cpp"

#include <cassert>

int main()
{
    sirkit::MeldableHeap<int> a, b;
    a.push(7);
    a.push(2);
    b.push(9);
    a.meld(b);
    assert(b.empty() && a.size() == 3 && a.top() == 9);
    a.pop();
    assert(a.top() == 7);
}
