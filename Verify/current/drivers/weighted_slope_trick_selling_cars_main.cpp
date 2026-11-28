// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>
#include "Verify/current/common/fast_io.hpp"

int main()
{
    int buyers = 0, colors = 0;
    verify_in >> buyers >> colors;
    std::vector<std::pair<std::int64_t, bool>> events(buyers + colors);
    for (int i = 0; i < buyers + colors; ++i)
    {
        verify_in >> events[i].first;
        events[i].second = i < buyers;
    }
    std::sort(events.begin(), events.end());
    const auto span = events.back().first - events.front().first;
    for (int capacity = 1; capacity <= buyers; ++capacity)
    {
        WeightedSlopeTrick cost;
        // 始点の余分な在庫・不足を1個減らす費用は、色の全幅以下で補正できる。
        cost.add_abs(0, span + 1);
        auto previous = events.front().first;
        for (const auto &[position, demand] : events)
        {
            cost.add_abs(0, position - previous);
            if (demand)
            {
                cost.shift(1);
            }
            else
            {
                cost.shift(-capacity, 0);
            }
            previous = position;
        }
        verify_out << cost.eval(0) << '\n';
    }
}
