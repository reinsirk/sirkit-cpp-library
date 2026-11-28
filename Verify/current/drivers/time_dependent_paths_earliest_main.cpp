// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{}, source{}, target{};
    verify_in >> n >> m >> source >> target;
    std::vector<std::pair<int, int>> edges;
    edges.reserve(2 * m);
    std::vector<std::pair<std::int64_t, std::int64_t>> schedule(m);
    for (auto &[length, interval] : schedule)
    {
        int u{}, v{};
        verify_in >> u >> v >> length >> interval;
        edges.emplace_back(u - 1, v - 1);
        edges.emplace_back(v - 1, u - 1);
    }
    const auto arrival = [&](std::size_t edge, std::int64_t ready) -> std::optional<std::int64_t>
    {
        const auto [length, interval] = schedule[edge / 2];
        const auto remainder = ready % interval;
        return ready + (remainder == 0 ? 0 : interval - remainder) + length;
    };
    const auto times = sirkit::earliest_arrival_times(n, edges, source - 1, 0, arrival);
    verify_out << times[target - 1].value_or(-1) << '\n';
}
