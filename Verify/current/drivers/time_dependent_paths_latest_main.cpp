// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

int main()
{
    struct Schedule
    {
        std::int64_t first, interval, count, duration;
    };
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    std::vector<Schedule> schedule(m);
    for (int i = 0; i < m; ++i)
    {
        auto &[first, interval, count, duration] = schedule[i];
        auto &[u, v] = edges[i];
        verify_in >> first >> interval >> count >> duration >> u >> v;
        --u;
        --v;
    }
    const auto departure = [&](std::size_t edge, std::int64_t deadline) -> std::optional<std::int64_t>
    {
        const auto [first, interval, count, duration] = schedule[edge];
        if (deadline < first + duration)
        {
            return std::nullopt;
        }
        const auto number = std::min(count - 1, (deadline - first - duration) / interval);
        return first + interval * number;
    };
    const auto times = sirkit::latest_departure_times(
        n, edges, n - 1, std::numeric_limits<std::int64_t>::max(), departure);
    for (int i = 0; i < n - 1; ++i)
    {
        if (times[i])
        {
            verify_out << *times[i] << '\n';
        }
        else
        {
            verify_out << "Unreachable\n";
        }
    }
}
