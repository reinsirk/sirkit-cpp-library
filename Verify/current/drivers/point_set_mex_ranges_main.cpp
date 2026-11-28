// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

int main()
{
    struct Event
    {
        int coordinate;
        std::size_t slot;
        int value;
    };

    std::size_t n{};
    verify_in >> n;
    std::vector<Event> events;
    events.reserve(2 * n);
    for (std::size_t i = 0; i < n; ++i)
    {
        int left{}, right{}, value{};
        verify_in >> left >> right >> value;
        events.push_back({left, i, value});
        events.push_back({right + 1, i, -1});
    }
    std::sort(events.begin(), events.end(), [](const Event &a, const Event &b) {
        return a.coordinate < b.coordinate;
    });

    sirkit::PointSetMex active(std::vector<int>(n, -1));
    std::size_t q{}, next = 0;
    verify_in >> q;
    for (std::size_t i = 0; i < q; ++i)
    {
        int coordinate{};
        verify_in >> coordinate;
        while (next < events.size() && events[next].coordinate <= coordinate)
        {
            const auto &event = events[next++];
            active.set(event.slot, event.value);
        }
        verify_out << active.mex() << '\n';
    }
}
