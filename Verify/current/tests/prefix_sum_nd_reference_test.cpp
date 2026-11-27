// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_PREFIX_SUM_ND_TEST_SOURCE
#define SIRKIT_PREFIX_SUM_ND_TEST_SOURCE "../../../Algorithm/Prefix_Sum_ND.cpp"
#endif
#include SIRKIT_PREFIX_SUM_ND_TEST_SOURCE
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace
{
template <std::size_t Dimensions>
void exhaustive_boxes(const std::array<std::size_t, Dimensions> &shape)
{
    using Sum = sirkit::CumulativeSumND<std::int64_t, Dimensions>;
    using Index = typename Sum::Index;
    std::size_t cells = 1;
    for (std::size_t length : shape)
    {
        cells *= length;
    }
    Sum sums(shape);
    assert(sums.shape() == shape);
    const auto point_at = [&shape](std::size_t flat)
    {
        Index point{};
        for (std::size_t d = Dimensions; d > 0; --d)
        {
            point[d - 1] = flat % shape[d - 1];
            flat /= shape[d - 1];
        }
        return point;
    };
    std::vector<std::int64_t> values(cells);
    for (std::size_t i = 0; i < cells; ++i)
    {
        values[i] = (static_cast<std::int64_t>((i * i + i * 3) % 13) - 4) * 1'000'000'000;
        sums.add(point_at(i), values[i] - 1);
        sums.add(point_at(i), 1);
    }
    sums.build();

    // 頂点の組でなく、元の配列の各セルを調べて期待値を作る。
    Index left{}, right{};
    const auto visit = [&](auto &self, std::size_t axis) -> void
    {
        if (axis != Dimensions)
        {
            for (left[axis] = 0; left[axis] <= shape[axis]; ++left[axis])
            {
                for (right[axis] = left[axis]; right[axis] <= shape[axis]; ++right[axis])
                {
                    self(self, axis + 1);
                }
            }
            return;
        }
        std::int64_t expected = 0;
        for (std::size_t i = 0; i < cells; ++i)
        {
            const Index point = point_at(i);
            bool inside = true;
            for (std::size_t d = 0; d < Dimensions; ++d)
            {
                inside = inside && left[d] <= point[d] && point[d] < right[d];
            }
            if (inside)
            {
                expected += values[i];
            }
        }
        assert(sums.query(left, right) == expected);
    };
    visit(visit, 0);
}

void scalar_api()
{
    sirkit::CumulativeSum2D<int> grid(3, 4);
    grid.add(0, 1, 5);
    grid.add(0, 1, -2);
    grid.add(2, 3, 7);
    grid.build();
    assert(grid.query(0, 1, 3, 4) == 10);
    assert(grid.query(0, 1, 1, 2) == 3);

    sirkit::CumulativeSum3D<int> cube(3, 1, 4);
    cube.add(0, 0, 2, 11);
    cube.add(2, 0, 1, -8);
    cube.build();
    assert(cube.query(0, 0, 0, 3, 1, 4) == 3);
    assert(cube.query(0, 0, 2, 3, 1, 4) == 11);

    sirkit::CumulativeSumND<int, 2> from_shape({1, 2});
    from_shape.add({0, 1}, 3);
    from_shape.build();
    assert(from_shape.query({0, 0}, {1, 2}) == 3);
}

void imos_api()
{
    sirkit::CumulativeSum2D<int> grid(3, 4);
    grid.imos(0, 0, 3, 4);
    grid.imos(1, 1, 2, 3, 7);
    grid.imos(1, 2, 3, 4, -3);
    grid.imos(3, 0, 3, 4, 100);
    grid.build();
    for (std::size_t i = 0; i < 3; ++i)
    {
        for (std::size_t j = 0; j < 4; ++j)
        {
            const int expected = 1 + 7 * (i == 1 && 1 <= j && j < 3) - 3 * (i >= 1 && j >= 2);
            assert(grid.imos_get(i, j) == expected);
        }
    }

    sirkit::CumulativeSum3D<int> cube(4, 3, 4);
    cube.imos(0, 0, 0, 4, 3, 4, 2);
    cube.imos(1, 0, 1, 3, 2, 3, -5);
    cube.build();
    for (std::size_t i = 0; i < 4; ++i)
    {
        for (std::size_t j = 0; j < 3; ++j)
        {
            for (std::size_t k = 0; k < 4; ++k)
            {
                const int expected = 2 - 5 * (1 <= i && i < 3 && j < 2 && 1 <= k && k < 3);
                assert(cube.imos_get(i, j, k) == expected);
            }
        }
    }

    sirkit::CumulativeSumND<int, 4> four({2, 2, 2, 2});
    four.imos({0, 0, 0, 0}, {2, 2, 2, 2}, 4);
    four.imos({0, 0, 0, 0}, {1, 1, 1, 1});
    four.build();
    for (std::size_t mask = 0; mask < 16; ++mask)
    {
        const std::array point{(mask >> 3) & 1, (mask >> 2) & 1, (mask >> 1) & 1, mask & 1};
        assert(four.imos_get(point) == 4 + (mask == 0));
    }
}

#ifndef SKIP_RUNTIME_CHECKS
template <class Error, class Function>
void expect_exception(Function operation)
{
    try
    {
        operation();
    }
    catch (const Error &)
    {
        return;
    }
    assert(false);
}
#endif
} // namespace

int main()
{
    exhaustive_boxes<1>({3});
    exhaustive_boxes<2>({2, 3});
    exhaustive_boxes<3>({2, 1, 3});
    exhaustive_boxes<4>({2, 2, 1, 3});
    scalar_api();
    imos_api();

    const std::size_t largest = std::numeric_limits<std::size_t>::max();
    sirkit::CumulativeSum3D<int> empty(0, largest, 2);
    assert((empty.shape() == std::array<std::size_t, 3>{0, largest, 2}));
    empty.imos(0, 1, 1, 0, largest, 2);
    empty.build();
    assert(empty.query(0, 1, 1, 0, largest, 2) == 0);

#ifndef SKIP_RUNTIME_CHECKS
    sirkit::CumulativeSum2D<int> grid(2, 2);
    expect_exception<std::length_error>([&]
                                        {
                                            sirkit::CumulativeSum2D<int> bad(largest, 1);
                                        });
    expect_exception<std::length_error>([&]
                                        {
                                            sirkit::CumulativeSum2D<int> bad(largest / 16, 100);
                                        });
    expect_exception<std::out_of_range>([&]
                                        {
                                            grid.add(0, 2, 1);
                                        });
    expect_exception<std::out_of_range>([&]
                                        {
                                            grid.imos(0, 0, 2, 3);
                                        });
    expect_exception<std::logic_error>([&]
                                       {
                                           grid.query(0, 0, 1, 1);
                                       });
    grid.add(1, 1, 5);
    grid.build();
    expect_exception<std::logic_error>([&]
                                       {
                                           grid.build();
                                       });
    expect_exception<std::logic_error>([&]
                                       {
                                           grid.add(0, 0, 1);
                                       });
    expect_exception<std::logic_error>([&]
                                       {
                                           grid.imos(0, 0, 1, 1);
                                       });
    expect_exception<std::out_of_range>([&]
                                        {
                                            grid.query(2, 0, 1, 2);
                                        });
    expect_exception<std::out_of_range>([&]
                                        {
                                            grid.imos_get(2, 0);
                                        });
    expect_exception<std::out_of_range>([&]
                                        {
                                            empty.query(0, 0, 0, 0, 1, 3);
                                        });
    assert(grid.query(0, 0, 2, 2) == 5);
#endif
}
