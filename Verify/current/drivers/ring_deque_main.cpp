// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

int main()
{
    int q = 0;
    verify_in >> q;
    sirkit::RingDeque<int> deque;
    deque.reserve(q);
    for (int step = 0; step < q; ++step)
    {
        int type = 0, value = 0;
        verify_in >> type;
        switch (type)
        {
        case 0:
            verify_in >> value;
            deque.push_front(value);
            break;
        case 1:
            verify_in >> value;
            deque.push_back(value);
            break;
        case 2:
            deque.pop_front();
            break;
        case 3:
            deque.pop_back();
            break;
        default:
            verify_in >> value;
            verify_out << deque[value] << '\n';
            break;
        }
    }
}
