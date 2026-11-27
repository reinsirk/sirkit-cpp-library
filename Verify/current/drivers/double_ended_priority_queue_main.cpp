// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n, q;
    verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    DoubleEndedPriorityQueue<int> queue(std::move(values));
    while (q-- != 0)
    {
        int kind;
        verify_in >> kind;
        if (kind == 0)
        {
            int value;
            verify_in >> value;
            queue.push(value);
        }
        else
        {
            verify_out << (kind == 1 ? queue.pop_min() : queue.pop_max()) << '\n';
        }
    }
}
