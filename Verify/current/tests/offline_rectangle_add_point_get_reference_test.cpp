// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Offline_Rectangle_Add_Point_Get.cpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <variant>
#include <vector>

int main()
{
    using sirkit::offline_rectangle_add_point_get;
    using sirkit::RectangleAdd;
    using sirkit::RectangleAddPointGetOperation;
    using sirkit::RectanglePointGet;
    const auto brute_force = [](std::vector<RectangleAdd> rectangles,
                                const std::vector<RectangleAddPointGetOperation> &operations)
    {
        std::vector<std::int64_t> answers;
        for (const auto &operation : operations)
        {
            if (const auto *add = std::get_if<RectangleAdd>(&operation))
            {
                rectangles.push_back(*add);
                continue;
            }
            const auto [x, y] = std::get<RectanglePointGet>(operation);
            std::uint64_t sum = 0;
            for (const auto &[left, bottom, right, top, weight] : rectangles)
            {
                if (left <= x && x < right && bottom <= y && y < top)
                {
                    sum += static_cast<std::uint64_t>(weight);
                }
            }
            answers.push_back(std::bit_cast<std::int64_t>(sum));
        }
        return answers;
    };

    assert(offline_rectangle_add_point_get({}, {}).empty());
    {
        const std::vector<RectangleAddPointGetOperation> operations{RectanglePointGet{2, 4}, RectanglePointGet{0, 0}};
        assert((offline_rectangle_add_point_get({}, operations) == std::vector<std::int64_t>{0, 0}));
    }

    {
        const std::vector<RectangleAdd> initial{{-1, -1, 1, 1, 3}, {-1, -1, 1, 1, 3}};
        const std::vector<RectangleAddPointGetOperation> operations{
            RectanglePointGet{-1, -1},
            RectanglePointGet{1, -1},
            RectanglePointGet{-1, 1},
            RectangleAdd{-1, -1, 1, 1, -10},
            RectanglePointGet{-1, -1},
            RectangleAdd{-1, -1, 1, -1, 20},
            RectangleAdd{0, -3, 0, 4, 21},
            RectangleAdd{-1, -1, 1, 1, 0},
            RectanglePointGet{-1, -1},
            RectanglePointGet{1, 1},
            RectangleAdd{-1, -1, 1, 1, 2},
        };
        assert((offline_rectangle_add_point_get(initial, operations) == std::vector<std::int64_t>{6, 0, 0, -4, -4, 0}));
    }
    {
        constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
        constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
        const std::vector<RectangleAdd> initial{{kMin, kMin, kMax, kMax, kMin}};
        const std::vector<RectangleAddPointGetOperation> operations{
            RectanglePointGet{kMin, kMin},
            RectangleAdd{kMin, kMin, kMax, kMax, kMin},
            RectanglePointGet{0, 0},
            RectangleAdd{kMin, kMin, kMax, kMax, kMax},
            RectangleAdd{kMin, kMin, kMax, kMax, 2},
            RectanglePointGet{kMax - 1, kMax - 1},
            RectanglePointGet{kMax, 0},
            RectanglePointGet{0, kMax},
        };
        assert((offline_rectangle_add_point_get(initial, operations) ==
                std::vector<std::int64_t>{kMin, 0, kMin + 1, 0, 0}));
    }

    std::mt19937 random(51977);
    const auto coordinate = [&]()
    {
        return static_cast<int>(random() % 13) - 6;
    };
    const auto rectangle = [&]()
    {
        const auto [left, right] = std::minmax({coordinate(), coordinate()});
        const auto [bottom, top] = std::minmax({coordinate(), coordinate()});
        return RectangleAdd{left, bottom, right, top, static_cast<int>(random() % 33) - 16};
    };
    for (unsigned trial = 0; trial < 5000; ++trial)
    {
        std::vector<RectangleAdd> initial;
        std::vector<RectangleAddPointGetOperation> operations;
        for (unsigned i = 0, n = random() % 14; i < n; ++i)
        {
            initial.push_back(rectangle());
        }
        for (unsigned i = 0, q = random() % 80; i < q; ++i)
        {
            if (random() % 3 == 0)
            {
                operations.emplace_back(rectangle());
            }
            else
            {
                operations.emplace_back(RectanglePointGet{coordinate(), coordinate()});
            }
        }
        assert(offline_rectangle_add_point_get(initial, operations) == brute_force(initial, operations));
    }

    assert(sirkit::detail::CheckedRectangleAddPointGetEventCount(1, 2, 4, 10) == 10);
    for (const auto &count : {std::vector<std::size_t>{0, 0, 11}, {4, 0, 4}, {2, 2, 4}})
    {
        try
        {
            (void)sirkit::detail::CheckedRectangleAddPointGetEventCount(count[0], count[1], count[2], 10);
            assert(false);
        }
        catch (const std::length_error &)
        {
        }
    }

#ifndef SKIP_RUNTIME_CHECKS
    const RectangleAdd reverse_x{1, -1, -1, 1, 5}, reverse_y{-1, 1, 1, -1, 5};
    try
    {
        offline_rectangle_add_point_get({&reverse_x, 1}, {});
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
    try
    {
        const RectangleAddPointGetOperation operation{reverse_y};
        offline_rectangle_add_point_get({}, {&operation, 1});
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
