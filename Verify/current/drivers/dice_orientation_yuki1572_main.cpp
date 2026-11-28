// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

int main()
{
    using sirkit::DiceFace;
    using sirkit::DiceOrientation;
    using sirkit::DiceRoll;
    int height{}, width{}, start_y{}, start_x{}, goal_y{}, goal_x{};
    verify_in >> height >> width >> start_y >> start_x >> goal_y >> goal_x;
    const auto stride = width + 2;
    const auto source = start_y * stride + start_x;
    const auto goal = goal_y * stride + goal_x;
    std::vector<char> open((height + 2) * stride, false);
    for (int y = 1; y <= height; ++y)
    {
        std::string row;
        verify_in >> row;
        for (int x = 1; x <= width; ++x)
        {
            open[y * stride + x] = row[x - 1] == '.';
        }
    }
    constexpr auto kOrientations = DiceOrientation::all();
    constexpr std::array kRolls{DiceRoll::North, DiceRoll::South, DiceRoll::East, DiceRoll::West};
    const std::array delta{-stride, stride, 1, -1};
    std::vector<std::uint32_t> seen(open.size());
    std::vector<std::uint32_t> pending;
    pending.reserve(12 * open.size());
    pending.push_back(static_cast<std::uint32_t>(source) << 5);
    seen[source] = 1;
    std::size_t head = 0;
    for (int steps = 0; head < pending.size(); ++steps)
    {
        const auto breadth_end = pending.size();
        while (head < breadth_end)
        {
            const auto state = pending[head++];
            const auto at = static_cast<int>(state >> 5);
            const auto &orientation = kOrientations[state & 31];
            if (at == goal && orientation.original_face(DiceFace::Top) == DiceFace::Top)
            {
                verify_out << steps << '\n';
                return 0;
            }
            for (std::size_t dir = 0; dir < delta.size(); ++dir)
            {
                const auto next = at + delta[dir];
                if (!open[next])
                {
                    continue;
                }
                const auto id = orientation.rolled(kRolls[dir]).id();
                const auto mask = std::uint32_t{1} << id;
                if ((seen[next] & mask) == 0)
                {
                    seen[next] |= mask;
                    pending.push_back((static_cast<std::uint32_t>(next) << 5) | static_cast<std::uint32_t>(id));
                }
            }
        }
    }
    verify_out << -1 << '\n';
}
