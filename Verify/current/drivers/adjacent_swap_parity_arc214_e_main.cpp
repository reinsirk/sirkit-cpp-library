// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

namespace
{
std::uint64_t Cost(std::optional<std::uint64_t> distance, std::uint64_t group, std::uint64_t parity)
{
    constexpr auto kImpossible = std::numeric_limits<std::uint64_t>::max();
    if (!distance || (parity == 1 && (group & 1u) == 0))
    {
        return kImpossible;
    }
    const auto cost = (*distance + group - 1) / group;
    return cost + ((group & 1u) != 0 && (cost & 1u) != parity);
}
} // namespace

int main()
{
    std::size_t tests{};
    verify_in >> tests;
    for (std::size_t test = 0; test < tests; ++test)
    {
        std::size_t n{};
        std::uint64_t group{};
        verify_in >> n >> group;
        std::vector<int> from(n), to(n);
        for (auto &value : from)
        {
            verify_in >> value;
        }
        for (auto &value : to)
        {
            verify_in >> value;
        }
        const auto lengths = sirkit::adjacent_swap_distances_by_parity(from, to);
        const auto best = std::min(Cost(lengths.even, group, 0), Cost(lengths.odd, group, 1));
        if (best == std::numeric_limits<std::uint64_t>::max())
        {
            verify_out << -1 << '\n';
        }
        else
        {
            verify_out << best << '\n';
        }
    }
}
