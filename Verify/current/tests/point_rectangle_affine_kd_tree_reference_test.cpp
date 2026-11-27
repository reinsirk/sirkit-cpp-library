// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Point_Rectangle_Affine_KD_Tree.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;
using Tree = sirkit::PointRectangleAffineKDTree<kMod>;

std::uint32_t Normalize(std::int64_t value)
{
    const auto r = value % static_cast<std::int64_t>(kMod);
    return static_cast<std::uint32_t>((r + kMod) % kMod);
}

bool Inside(Tree::Point point, Tree::Rectangle rectangle)
{
    return rectangle.left <= point.x && point.x < rectangle.right &&
           rectangle.bottom <= point.y && point.y < rectangle.top;
}
}

int main()
{
    Tree empty;
    assert(empty.size() == 0 && empty.active_count() == 0 && empty.all_sum() == 0);
    assert(empty.range_sum({0, 0, 1, 1}) == 0);
    empty.range_affine({0, 0, 1, 1}, 12, -5);

    Tree pending(std::array{Tree::Point{0, 0}, Tree::Point{0, 0}});
    pending.set(0, 4);
    pending.range_affine({0, 0, 1, 1}, 1, 5);
    pending.set(1, 3);
    assert(pending.all_sum() == 12);

    constexpr auto kLow = std::numeric_limits<std::int64_t>::min();
    constexpr auto kHigh = std::numeric_limits<std::int64_t>::max();
    const std::array<Tree::Point, 6> points{
        Tree::Point{kLow, kLow},
        {-1, 0},
        {-1, 0},
        {-1, 0},
        {0, kHigh},
        {0, 0},
    };
    Tree tree(points);
    assert(tree.size() == 6 && !tree.is_active(0));
    tree.range_affine({kLow, kLow, kHigh, kHigh}, 1, -19);
    tree.set(0, 5);
    tree.set(1, 4);
    tree.set(4, 100);
    tree.range_affine({kLow, kLow, kHigh, kHigh}, 2, 3);
    tree.range_affine({kLow, kLow, kHigh, kHigh}, 3, 7);
    tree.set(2, 11);
    assert(!tree.is_active(3) && tree.is_active(2));
    assert(tree.active_count() == 4);
    assert(tree.range_sum({kLow, kLow, -1, kHigh}) == 46);
    assert(tree.range_sum({-1, 0, 0, 1}) == 51);
    assert(tree.all_sum() == 197);
    tree.range_affine({-1, 0, 0, 1}, 0, 3);
    tree.set(3, 17);
    assert(tree.range_sum({-1, 0, 0, 1}) == 23);
    tree.set(1, -1);
    assert(tree.range_sum({-1, 0, 0, 1}) == 19);
    tree.range_affine({-1, 0, 0, 1}, 1, 0);
    tree.range_affine({-1, 1, 0, 1}, 0, 5);
    tree.range_affine({-1, 0, -1, 1}, 0, 5);
    assert(tree.range_sum({-1, 0, 0, 1}) == 19);
    assert(tree.range_sum({-1, 0, -1, 3}) == 0);
    assert(tree.range_sum({-1, 1, 0, 1}) == 0);

    auto copy = tree;
    copy.set(0, 93);
    assert(tree.range_sum({kLow, kLow, -1, kHigh}) == 46);
    auto moved = std::move(copy);
    assert(copy.size() == 0);
    auto &move_alias = moved;
    moved = std::move(move_alias);
    assert(moved.range_sum({kLow, kLow, -1, kHigh}) == 93);
    copy = moved;
    const auto &copy_alias = copy;
    copy = copy_alias;
    assert(copy.all_sum() == moved.all_sum());
    moved = std::move(copy);
    assert(copy.size() == 0 && moved.size() == 6);

    using LargeMod = sirkit::PointRectangleAffineKDTree<std::numeric_limits<std::uint32_t>::max()>;
    constexpr auto kLargeMod = std::numeric_limits<std::uint32_t>::max();
    const std::array<LargeMod::Point, 5> large_points{};
    LargeMod large(large_points);
    large.set(0, kLargeMod - 1);
    for (int i = 1; i < large.size(); ++i)
    {
        large.set(i, 0);
    }
    large.range_affine({0, 0, 1, 1}, kLargeMod - 1, kLargeMod - 1);
    assert(large.all_sum() == kLargeMod - 4);
    large.set(1, 0);
    assert(large.range_sum({0, 0, 1, 1}) == kLargeMod - 3);

    using UnitMod = sirkit::PointRectangleAffineKDTree<1>;
    UnitMod unit(std::array{UnitMod::Point{0, 0}});
    unit.set(0, kHigh);
    unit.range_affine({0, 0, 1, 1}, kLow, kHigh);
    assert(unit.is_active(0) && unit.all_sum() == 0);

    std::mt19937_64 rng(880593);
    const auto coord = [&]
    {
        return static_cast<std::int64_t>(rng() % 23) - 11;
    };
    const auto coefficient = [&]
    {
        return static_cast<std::int64_t>(rng() % (2 * std::uint64_t{kMod} + 101)) - kMod;
    };
    for (int trial = 0; trial < 500; ++trial)
    {
        const int n = trial % 73 + 1;
        std::vector<Tree::Point> positions(n);
        for (auto &p : positions)
        {
            p = {coord(), (trial % 2 == 0 ? coord() : 2)};
        }
        Tree candidate(positions);
        std::vector<std::optional<std::uint32_t>> values(n);
        for (int step = 0; step < 250; ++step)
        {
            const auto [left, right] = std::minmax({coord(), coord()});
            const auto [bottom, top] = std::minmax({coord(), coord()});
            const Tree::Rectangle rectangle = step % 9 == 0 ? Tree::Rectangle{-13, -13, 13, 13}
                                                            : Tree::Rectangle{left, bottom, right, top};
            const int id = static_cast<int>(rng() % n);
            if (step % 4 == 0)
            {
                const auto value = coefficient();
                candidate.set(id, value);
                values[id] = Normalize(value);
            }
            if (step % 4 == 2 || step % 7 == 0)
            {
                const auto a = coefficient();
                const auto b = coefficient();
                candidate.range_affine(rectangle, a, b);
                for (int i = 0; i < n; ++i)
                {
                    if (values[i] && Inside(positions[i], rectangle))
                    {
                        values[i] = static_cast<std::uint32_t>(
                            (std::uint64_t{Normalize(a)} * *values[i] + Normalize(b)) % kMod);
                    }
                }
            }

            std::uint64_t expected = 0, all = 0;
            int active = 0;
            for (int i = 0; i < n; ++i)
            {
                if (values[i])
                {
                    all += *values[i];
                    ++active;
                    if (Inside(positions[i], rectangle))
                    {
                        expected += *values[i];
                    }
                }
            }
            assert(candidate.range_sum(rectangle) == expected % kMod);
            assert(candidate.all_sum() == all % kMod && candidate.active_count() == active);
            assert(candidate.is_active(id) == values[id].has_value());
        }
    }

    assert(sirkit::detail::CheckedPointRectangleAffineNodeCount(5, 9) == 9);
    for (const auto &[count, capacity] : {
             std::pair{std::size_t{5}, std::size_t{8}},
             std::pair{static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1, std::size_t{100}},
         })
    {
        try
        {
            (void)sirkit::detail::CheckedPointRectangleAffineNodeCount(count, capacity);
            assert(false);
        }
        catch (const std::length_error &)
        {
        }
    }

#ifndef SKIP_RUNTIME_CHECKS
    for (int bad : {-1, 6})
    {
        try
        {
            tree.set(bad, 1);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
    for (const auto &rectangle : {Tree::Rectangle{2, 0, 1, 2}, Tree::Rectangle{0, 2, 1, 1}})
    {
        try
        {
            (void)tree.range_sum(rectangle);
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
#endif
}
