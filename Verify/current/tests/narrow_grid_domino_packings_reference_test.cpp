// SPDX-License-Identifier: CC0-1.0
#include "Algorithm/Narrow_Grid_Domino_Packings.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
std::pair<std::uint32_t, std::uint32_t> brute(int height, int width)
{
    std::vector<std::pair<unsigned, unsigned>> edges;
    const int n = height * width;
    for (int i = 0; i < n; ++i)
    {
        if (i % width != width - 1)
        {
            edges.emplace_back(1U << i, 1U << (i + 1));
        }
        if (i + width < n)
        {
            edges.emplace_back(1U << i, 1U << (i + width));
        }
    }
    const auto all = (1U << n) - 1;
    std::uint32_t perfect = 0, maximal = 0;
    const auto visit = [&](const auto &self, int start, unsigned covered) -> void
    {
        bool can_extend = false;
        for (int i = 0; i < static_cast<int>(edges.size()); ++i)
        {
            const auto [u, v] = edges[i];
            if ((covered & (u | v)) == 0)
            {
                can_extend = true;
                if (i >= start)
                {
                    self(self, i + 1, covered | u | v);
                }
            }
        }
        perfect += covered == all;
        maximal += !can_extend;
    };
    visit(visit, 0, 0);
    return {perfect, maximal};
}

template <class Function>
void expect_invalid(Function function)
{
    try
    {
        function();
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
}
}

int main()
{
    using sirkit::DominoPackingMode;
    using sirkit::narrow_grid_domino_packings;
    for (int h = 0; h <= 5; ++h)
    {
        for (int w = 0; w <= 5; ++w)
        {
            if (h * w > 16)
            {
                continue;
            }
            const auto [perfect, maximal] = brute(h, w);
            for (auto mod : std::array<std::uint32_t, 4>{1, 2, 73, 4294967295U})
            {
                assert(narrow_grid_domino_packings(h, w, DominoPackingMode::kPerfect, mod) == perfect % mod);
                assert(narrow_grid_domino_packings(h, w, DominoPackingMode::kMaximal, mod) == maximal % mod);
            }
        }
    }
    const auto wide = std::numeric_limits<std::uint64_t>::max();
    assert(narrow_grid_domino_packings(1, wide, DominoPackingMode::kPerfect) == 0);
    assert(narrow_grid_domino_packings(1, wide - 1, DominoPackingMode::kPerfect) == 1);
    expect_invalid([]
                   {
                       (void)narrow_grid_domino_packings(-1, 1, DominoPackingMode::kPerfect);
                   });
    expect_invalid([]
                   {
                       (void)narrow_grid_domino_packings(6, 1, DominoPackingMode::kMaximal);
                   });
    expect_invalid([]
                   {
                       (void)narrow_grid_domino_packings(1, 1, DominoPackingMode::kMaximal, 0);
                   });
}
