// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <tuple>
#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<std::tuple<int, int, int>> events;
    events.reserve(static_cast<std::size_t>(2 * n));
    for (int i = 0; i < n; ++i)
    {
        int left, right, value;
        verify_in >> left >> right >> value;
        if (value <= n)
        {
            events.emplace_back(left, value, 1);
            events.emplace_back(right + 1, value, -1);
        }
    }
    std::sort(events.begin(), events.end());

    sirkit::IntervalSet<int> active;
    std::vector<int> frequency(static_cast<std::size_t>(n + 1));
    std::size_t next = 0;
    int q;
    verify_in >> q;
    for (int i = 0; i < q; ++i)
    {
        int position;
        verify_in >> position;
        while (next < events.size() && std::get<0>(events[next]) <= position)
        {
            const auto [coordinate, value, delta] = events[next++];
            auto &count = frequency[static_cast<std::size_t>(value)];
            if (count == 0)
            {
                active.add(value, value + 1);
            }
            count += delta;
            if (count == 0)
            {
                active.erase(value, value + 1);
            }
        }
        verify_out << active.mex() << '\n';
    }
}
