// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Compressed_Fenwick_Tree_2D.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <map>
#include <random>
#include <stdexcept>
#include <vector>

using Tree = sirkit::CompressedFenwickTree2D<>;

void CheckRandom()
{
    std::mt19937 rng(3514452);
    Tree empty;
    assert(empty.empty() && empty.size() == 0);
    assert(!empty.add({0, 0}, 5) && !empty.point_id({0, 0}));
    assert(empty.sum(-1, -1, 1, 1) == 0 && empty.prefix_sum(1, 1) == 0);

    for (int trial = 0; trial < 7; ++trial)
    {
        std::vector<Tree::Point> points;
        std::map<Tree::Point, std::int64_t> values;
        for (int i = 0; i < 110; ++i)
        {
            const auto x = static_cast<int>(rng() % 21) - 10;
            const auto y = static_cast<int>(rng() % 17) - 8;
            points.emplace_back(x, y);
            points.emplace_back(x, y);
            values.emplace(Tree::Point{x, y}, 0);
        }
        Tree tree(points);
        assert(tree.size() == values.size() && !tree.empty());
        assert(!tree.point_id({-100, -100}) && !tree.point_id({100, 100}));
        assert(!tree.add({points.front().first, 100}, 100));

        std::size_t id = 0;
        for (const auto &[point, value] : values)
        {
            assert(tree.point_id(point) == id++);
            assert(value == 0);
        }
        for (int i = 0; i < 2400; ++i)
        {
            const auto point = points[rng() % points.size()];
            const auto addition = static_cast<int>(rng() % 901) - 450;
            if (i % 2)
            {
                assert(tree.add(point, addition));
            }
            else
            {
                tree.add(*tree.point_id(point), addition);
            }
            values[point] += addition;
            auto [l, r] = std::minmax({static_cast<int>(rng() % 31) - 15, static_cast<int>(rng() % 31) - 15});
            auto [b, t] = std::minmax({static_cast<int>(rng() % 25) - 12, static_cast<int>(rng() % 25) - 12});
            std::int64_t answer = 0, prefix = 0;
            for (auto [xy, value] : values)
            {
                const auto [x, y] = xy;
                if (l <= x && x < r && b <= y && y < t)
                {
                    answer += value;
                }
                if (x < r && y < t)
                {
                    prefix += value;
                }
            }
            assert(tree.sum(l, b, r, t) == answer);
            assert(tree.prefix_sum(r, t) == prefix);
        }
    }
}

int main()
{
    CheckRandom();
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    std::vector<Tree::Point> points{{kMax, kMin}, {kMin, kMax}, {kMin, kMin}, {0, 0}, {0, 0}};
    Tree tree(points);
    assert(tree.add({kMin, kMin}, kMax));
    assert(tree.add({kMin, kMin}, kMax));
    assert(tree.add({0, 0}, kMin));
    assert(tree.add({kMax, kMin}, 9));
    assert(tree.add({kMin, kMax}, 9));
    assert(tree.sum(kMin, kMin, 0, 0) == -2);
    assert(tree.sum(kMin, kMin, kMax, kMax) == kMax - 1);
    assert(tree.prefix_sum(1, 1) == kMax - 1);

    auto independent = tree;
    independent.add(*tree.point_id({0, 0}), 10);
    assert(tree.sum(0, 0, 1, 1) == kMin && independent.sum(0, 0, 1, 1) == kMin + 10);
    sirkit::CompressedFenwickTree2D<std::uint8_t> byte(points);
    byte.add(*byte.point_id({0, 0}), 255);
    byte.add(*byte.point_id({0, 0}), 7);
    assert(byte.sum(0, 0, 1, 1) == 6);
#ifndef SKIP_RUNTIME_CHECKS
    bool bad_id = false, wrong_x = false, wrong_y = false;
    try
    {
        tree.add(tree.size(), 0);
    }
    catch (const std::out_of_range &)
    {
        bad_id = true;
    }
    try
    {
        (void)tree.sum(1, 0, 0, 1);
    }
    catch (const std::invalid_argument &)
    {
        wrong_x = true;
    }
    try
    {
        (void)tree.sum(0, 1, 1, 0);
    }
    catch (const std::invalid_argument &)
    {
        wrong_y = true;
    }
    assert(bad_id && wrong_x && wrong_y);
#endif
}
