// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <vector>

struct Event
{
    int row, left, right, delta;
};

int main()
{
    int n{}, q{};
    verify_in >> n >> q;
    std::vector<Event> events;
    events.reserve(2 * q);
    for (int i = 0; i < q; ++i)
    {
        int top{}, left{}, bottom{}, right{};
        verify_in >> top >> left >> bottom >> right;
        events.push_back({top - 1, left - 1, right, 1});
        events.push_back({bottom, left - 1, right, -1});
    }
    std::ranges::sort(events, {}, &Event::row);
    std::vector<std::uint64_t> weights(n);
    std::iota(weights.begin(), weights.end(), std::uint64_t{1});
    sirkit::RangeAddWeightedMinimum<int> data(weights);
    auto event = events.cbegin();
    constexpr std::uint64_t kMod = 998244353;
    std::uint64_t answer = 0;
    for (int row = 0; row < n; ++row)
    {
        for (; event != events.cend() && event->row == row; ++event)
        {
            data.add(event->left, event->right, event->delta);
        }
        const auto result = data.all_minimum().value();
        if (result.value == 0)
        {
            answer = (answer + (row + 1) * result.weight) % kMod;
        }
    }
    verify_out << answer << '\n';
}
