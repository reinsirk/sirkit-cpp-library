// SPDX-License-Identifier: CC0-1.0
#include "DataStructure/Grid_Connectivity_Frontier.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace
{
std::uint32_t brute(int h, int w)
{
    const int n = h * w;
    const unsigned goal = 1U << (n - 1);
    std::uint32_t result = 0;
    for (unsigned set = goal | 1U; set < (goal << 1); set += 2)
    {
        unsigned reached = 1;
        for (int round = 0; round < n; ++round)
        {
            const unsigned last = reached;
            for (int vertex = 0; vertex < n; ++vertex)
            {
                if ((reached & (1U << vertex)) == 0)
                {
                    continue;
                }
                const int row = vertex / w, column = vertex % w;
                if (row > 0)
                {
                    reached |= set & (1U << (vertex - w));
                }
                if (row + 1 < h)
                {
                    reached |= set & (1U << (vertex + w));
                }
                if (column > 0)
                {
                    reached |= set & (1U << (vertex - 1));
                }
                if (column + 1 < w)
                {
                    reached |= set & (1U << (vertex + 1));
                }
            }
            if (last == reached)
            {
                break;
            }
        }
        result += (reached & goal) != 0;
    }
    return result;
}

template <class Exception, class Function>
void expect_throw(Function function)
{
    try
    {
        function();
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
}

int main()
{
    using sirkit::grid_corner_connected_colorings;
    using sirkit::GridConnectivityFrontier;
    for (int h = 1; h <= 5; ++h)
    {
        for (int w = 1; w <= 5; ++w)
        {
            if (h * w > 16)
            {
                continue;
            }
            const auto count = brute(h, w);
            for (std::uint32_t mod : std::array<std::uint32_t, 4>{1, 3, 97, 4294967295U})
            {
                assert(grid_corner_connected_colorings(h, w, mod) == count % mod);
            }
        }
    }

    const GridConnectivityFrontier grid(3);
    auto upper = grid.place(0, 0, true, true);
    assert(grid.label(upper.state, 0) == grid.kMarkedGroup);
    auto separate = grid.place(upper.state, 2, true);
    assert(grid.label(separate.state, 2) == 3);
    auto ordinary_closed = grid.place(separate.state, 2, false);
    assert(ordinary_closed.state == upper.state && ordinary_closed.closed_group == 3);
    const auto next_upper = grid.place(separate.state, 0, true);
    const auto bridge = grid.place(next_upper.state, 1, true, true);
    auto merged = grid.place(bridge.state, 2, true);
    assert(merged.closed_group == 0);
    for (int i = 0; i < 3; ++i)
    {
        assert(grid.label(merged.state, i) == grid.kMarkedGroup);
    }
    assert(grid.place(upper.state, 0, false).closed_group == grid.kMarkedGroup);
    assert(grid.label(0, 1) == 0);
    auto lower = grid.place(0, 2, true, true);
    auto previous_only = grid.place(lower.state, 2, true, true);
    assert(previous_only.state == lower.state);

    const GridConnectivityFrontier wide(14);
    assert(wide.label(wide.place(0, 13, true).state, 13) == 14);
    expect_throw<std::invalid_argument>([]
                                        {
                                            (void)GridConnectivityFrontier(0);
                                        });
    expect_throw<std::invalid_argument>([]
                                        {
                                            (void)GridConnectivityFrontier(15);
                                        });
    expect_throw<std::out_of_range>([&]
                                    {
                                        (void)grid.label(0, -1);
                                    });
    expect_throw<std::out_of_range>([&]
                                    {
                                        (void)grid.place(0, 3, true);
                                    });
    expect_throw<std::invalid_argument>([&]
                                        {
                                            (void)grid.place(0, 0, false, true);
                                        });
    expect_throw<std::invalid_argument>([&]
                                        {
                                            (void)grid.place(upper.state, 2, true, true);
                                        });
    expect_throw<std::invalid_argument>([]
                                        {
                                            (void)grid_corner_connected_colorings(-1, 1);
                                        });
    expect_throw<std::invalid_argument>([]
                                        {
                                            (void)grid_corner_connected_colorings(1, 0);
                                        });
    expect_throw<std::invalid_argument>([]
                                        {
                                            (void)grid_corner_connected_colorings(1, 1, 0);
                                        });
}
