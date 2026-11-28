// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/Kd_Tree_Point_Extractor.cpp"
#include <cassert>
#include <random>
#include <string_view>

namespace
{
using Extractor = sirkit::KdTreePointExtractor;
using Point = Extractor::Point;

void test_limits()
{
    Extractor pairs({{0, 0}, {1, 0}});
    std::vector<std::size_t> unused;
    pairs.take_in_circle({-10, 0}, 1, unused);
    pairs.take_in_rectangle({-10, -10}, {-9, 10}, unused);
    assert(unused.capacity() == 0 && pairs.size() == 2);

    constexpr auto lo = std::numeric_limits<std::int32_t>::min();
    constexpr auto hi = std::numeric_limits<std::int32_t>::max();
    constexpr auto radius = std::numeric_limits<std::uint32_t>::max();
    Extractor points({{lo, lo}, {lo, hi}, {hi, lo}, {hi, hi}, {0, 0}, {lo, lo}});
    std::vector<std::size_t> out{90};
    points.take_in_circle({lo, lo}, radius, out);
    std::sort(out.begin() + 1, out.end());
    assert((out == std::vector<std::size_t>{90, 0, 1, 2, 4, 5}));
    assert(points.size() == 1);
    points.take_in_circle({lo, lo}, radius, out);
    assert(out.size() == 6);
    points.take_in_rectangle({hi, hi}, {hi, hi}, out);
    assert(out.back() == 3);
    assert(points.size() == 0);
    points.take_in_rectangle({lo, lo}, {hi, hi}, out);
    assert(out.size() == 7);
    Extractor empty({});
    empty.take_in_circle({lo, lo}, radius, out);
    assert(out.size() == 7);
}

void test_mixed(std::mt19937 &random)
{
    const auto coord = [&]
    {
        return static_cast<std::int32_t>(random() % 30) - 15;
    };
    for (int trial = 0; trial < 400; ++trial)
    {
        const auto n = random() % 120;
        std::vector<Point> input(n);
        for (auto &[x, y] : input)
        {
            x = coord();
            y = coord();
        }
        Extractor tree(input);
        std::vector<bool> remaining(n, true);
        std::vector<std::size_t> actual, expected;
        for (int query = 0; query < 40; ++query)
        {
            const Point center{coord(), coord()};
            const auto radius = static_cast<std::uint32_t>(random() % 11);
            const Point upper{center[0] + static_cast<int>(random() % 15),
                              center[1] + static_cast<int>(random() % 15)};
            const bool circle = query % 2 == 0;
            if (circle)
            {
                tree.take_in_circle(center, radius, actual);
            }
            else
            {
                tree.take_in_rectangle(center, upper, actual);
            }
            for (std::size_t i = 0; i < input.size(); ++i)
            {
                const auto [x, y] = input[i];
                const int dx = x - center[0], dy = y - center[1];
                const bool selected =
                    circle ? dx * dx + dy * dy <= static_cast<int>(radius * radius)
                           : center[0] <= x && x <= upper[0] && center[1] <= y &&
                                 y <= upper[1];
                if (selected && remaining[i])
                {
                    remaining[i] = false;
                    expected.push_back(i);
                }
            }
            assert(tree.size() + actual.size() == input.size());
            std::sort(actual.begin(), actual.end());
            std::sort(expected.begin(), expected.end());
            assert(actual == expected);
        }
        tree.take_in_rectangle({-20, -20}, {20, 20}, actual);
        std::sort(actual.begin(), actual.end());
        assert(actual.size() == input.size());
        assert(tree.size() == 0);
        for (std::size_t i = 0; i < actual.size(); ++i)
        {
            assert(actual[i] == i);
        }
    }
}

#ifndef SKIP_RUNTIME_CHECKS
void test_invalid(const Point upper)
{
    Extractor tree({{0, 0}});
    std::vector<std::size_t> out;
    bool rejected = false;
    try
    {
        tree.take_in_rectangle({0, 0}, upper, out);
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected && tree.size() == 1 && out.empty());
}
#endif
} // namespace

int main(int argc, char **argv)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        Extractor tree({{0, 0}});
        std::vector<std::size_t> out;
        tree.take_in_rectangle({1, 1}, {0, 0}, out);
        return 1;
    }
    test_invalid({-1, 0});
    test_invalid({0, -1});
#endif
    static_cast<void>(argc);
    static_cast<void>(argv);
    test_limits();
    std::mt19937 random(781642);
    test_mixed(random);
}
