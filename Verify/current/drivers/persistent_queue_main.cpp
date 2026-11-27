// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int q = 0;
    verify_in >> q;
    PersistentQueue<int> queue;
    std::vector<PersistentQueue<int>::Version> versions(q + 1);
    for (int i = 1; i <= q; ++i)
    {
        int type = 0, t = 0;
        verify_in >> type >> t;
        const auto source = versions[t + 1];
        if (type == 0)
        {
            int value = 0;
            verify_in >> value;
            versions[i] = queue.push(source, value);
        }
        else
        {
            verify_out << queue.front(source) << '\n';
            versions[i] = queue.pop(source);
        }
    }
}
