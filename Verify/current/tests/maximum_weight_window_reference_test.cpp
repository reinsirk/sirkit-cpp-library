// SPDX-License-Identifier: CC0-1.0
#include "Geometry/Maximum_Weight_Window.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using sirkit::WeightedWindowPoint;

std::int64_t score(std::span<const WeightedWindowPoint> points, std::int64_t x, std::int64_t y, int width, int height)
{
    std::int64_t total = 0;
    for (const auto &[px, py, weight] : points)
    {
        if (x <= px && px < x + width && y <= py && py < y + height)
        {
            total += weight;
        }
    }
    return total;
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
    using sirkit::maximum_weight_window;
    std::mt19937 random(342923);
    for (int trial = 0; trial < 1400; ++trial)
    {
        const int width = 1 + random() % 5, height = 1 + random() % 5;
        std::vector<WeightedWindowPoint> points(random() % 18);
        for (auto &[x, y, w] : points)
        {
            x = static_cast<int>(random() % 13) - 6;
            y = static_cast<int>(random() % 13) - 6;
            w = static_cast<int>(random() % 21) - 10;
        }
        const auto window = maximum_weight_window(points, width, height);
        std::int64_t largest = 0;
        for (int x = -12; x <= 7; ++x)
        {
            for (int y = -12; y <= 7; ++y)
            {
                largest = std::max(largest, score(points, x, y, width, height));
            }
        }
        assert(window.weight == largest);
        assert(score(points, window.left, window.bottom, width, height) == largest);
    }

    constexpr auto kMin = std::numeric_limits<std::int32_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int32_t>::max();
    constexpr auto kLargestWidth = std::numeric_limits<std::uint32_t>::max();
    const std::array<WeightedWindowPoint, 2> extremes{{{kMin, kMin, 9}, {kMax, kMax, 12}}};
    assert(maximum_weight_window(extremes, kLargestWidth, kLargestWidth).weight == 12);
    const std::array<WeightedWindowPoint, 4> simultaneous{{{0, 0, 100}, {0, 0, -100}, {2, 0, -1}, {2, 0, 1}}};
    assert(maximum_weight_window(simultaneous, 2, 1).weight == 0);
    constexpr auto kMaxWeight = std::numeric_limits<std::int64_t>::max();
    const std::array<WeightedWindowPoint, 1> heavy{{{0, 0, kMaxWeight}}};
    assert(maximum_weight_window(heavy, 1, 1).weight == kMaxWeight);
    const std::array<WeightedWindowPoint, 1> unrepresentable{{{0, 0, std::numeric_limits<std::int64_t>::min()}}};
    expect_throw<std::overflow_error>([&]
                                      {
                                          (void)maximum_weight_window(unrepresentable, 1, 1);
                                      });
    expect_throw<std::invalid_argument>([]
                                        {
                                            (void)maximum_weight_window({}, 0, 1);
                                        });
    expect_throw<std::invalid_argument>([]
                                        {
                                            (void)maximum_weight_window({}, 1, 0);
                                        });
    expect_throw<std::length_error>([]
                                    {
                                        sirkit::detail::CheckWindowPointCount(static_cast<std::size_t>(std::numeric_limits<int>::max()) / 2 + 1);
                                    });
    const auto padding = sirkit::detail::EmptyWindowWeight();
    assert(sirkit::detail::IncreaseWindowWeight(-1, padding) == padding);
}
