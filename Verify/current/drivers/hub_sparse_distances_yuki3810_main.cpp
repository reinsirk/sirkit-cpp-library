// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

int main()
{
    std::size_t n{}, m{}, queries{};
    verify_in >> n >> m >> queries;
    std::vector<std::uint64_t> gates(n);
    for (auto &gate : gates)
    {
        verify_in >> gate;
    }
    constexpr std::uint64_t kMaxWarp = 2000000000000000000;
    std::vector<sirkit::HubSparseDistances::Shortcut> shortcuts;
    shortcuts.reserve(m);
    for (std::size_t i = 1; i <= m; ++i)
    {
        int u{}, v{};
        std::uint64_t weight{};
        verify_in >> u >> v >> weight;
        if (weight == 0)
        {
            shortcuts.push_back({u - 1, v - 1, 0});
        }
        else if (i < std::numeric_limits<std::uint64_t>::digits && weight <= (kMaxWarp >> i))
        {
            shortcuts.push_back({u - 1, v - 1, weight << i});
        }
    }
    const sirkit::HubSparseDistances graph(gates, shortcuts);
    for (std::size_t i = 0; i < queries; ++i)
    {
        int from{}, to{};
        verify_in >> from >> to;
        verify_out << graph.distance(from - 1, to - 1) << '\n';
    }
}
