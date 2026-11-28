// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Time_Dependent_Paths.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}};
    const auto board = [](std::size_t, std::int64_t ready) -> std::optional<std::int64_t>
    {
        return ready <= 3 ? std::optional<std::int64_t>(5) : std::nullopt;
    };
    const auto arrive = sirkit::earliest_arrival_times(2, edges, 0, 1, board);
    assert(arrive[1] == 5);

    const auto reverse = [](std::size_t, std::int64_t deadline) -> std::optional<std::int64_t>
    {
        return 5 <= deadline ? std::optional<std::int64_t>(3) : std::nullopt;
    };
    assert(sirkit::latest_departure_times(2, edges, 1, 7, reverse)[0] == 3);
}
