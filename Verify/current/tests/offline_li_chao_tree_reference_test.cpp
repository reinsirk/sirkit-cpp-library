// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Offline_Li_Chao_Tree.cpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

using Tree = sirkit::OfflineLiChaoTree<std::int64_t>;
using Line = sirkit::OfflineLiChaoLine<std::int64_t>;

void check_limits()
{
    std::size_t count = 3;
    sirkit::detail::offline_li_chao_add_count(count, 2, 5);
    assert(count == 5 && sirkit::detail::offline_li_chao_time_base(3, 9) == 4);
    bool bad_capacity = false;
    try
    {
        sirkit::detail::offline_li_chao_add_count(count, 1, 5);
    }
    catch (const std::length_error &)
    {
        bad_capacity = true;
    }
    assert(bad_capacity);
    bad_capacity = false;
    try
    {
        static_cast<void>(sirkit::detail::offline_li_chao_time_base(5, 9));
    }
    catch (const std::length_error &)
    {
        bad_capacity = true;
    }
    assert(bad_capacity);
    Tree lines;
    assert(lines.solve().empty());
    auto id = lines.add_line({2, -1});
    lines.erase(id);
    assert(lines.query(-1) == 0);
    const auto empty_line = lines.add_segment(-1, -1, {0, -100});
    assert(lines.query(-1) == 1);
    const auto a = lines.add_segment(-1, 2, {3, -2});
    assert(lines.query(-1) == 2);
    assert(lines.query(2) == 3);
    lines.erase(empty_line);
    lines.add_line({0, -8});
    assert(lines.query(2) == 4);
    const std::vector<std::optional<std::int64_t>> expected{std::nullopt, std::nullopt, -5, std::nullopt, -8};
    assert(lines.solve() == expected);
    auto copied = lines;
    copied.erase(a);
    lines.erase(a);
    auto moved = std::move(copied);
    copied = Tree{};
    copied = moved;
    moved.query(2);
    assert(copied.solve() == expected);
    assert(moved.solve().back() == -8);
#ifndef SKIP_RUNTIME_CHECKS
    bool caught = false;
    try
    {
        lines.erase(a);
    }
    catch (const std::out_of_range &)
    {
        caught = true;
    }
    assert(caught);
    caught = false;
    try
    {
        lines.erase(100);
    }
    catch (const std::out_of_range &)
    {
        caught = true;
    }
    assert(caught);
    caught = false;
    try
    {
        lines.add_segment(5, 4, {2, 3});
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught);
    assert(lines.solve() == expected);
#endif
}

void check_independent()
{
    struct Record
    {
        Line line;
        std::int64_t first;
        std::int64_t last;
        bool active;
    };
    std::mt19937 random(8891);
    const auto number = [&]
    {
        return static_cast<std::int64_t>(random() % 61) - 30;
    };
    for (int trial = 0; trial < 500; ++trial)
    {
        Tree lines;
        std::vector<Record> all;
        std::vector<std::size_t> active;
        std::vector<std::optional<std::int64_t>> expected;
        for (int step = 0; step < 390; ++step)
        {
            const auto type = random() % 5;
            if (type < 2)
            {
                const Line line{number(), number()};
                const auto start = number();
                const auto finish = number();
                std::size_t id;
                if (type == 0)
                {
                    id = lines.add_line(line);
                    all.push_back({line, -100, 100, true});
                }
                else
                {
                    const auto [a, b] = std::minmax(start, finish);
                    id = lines.add_segment(a, b, line);
                    all.push_back({line, a, b, true});
                }
                assert(id + 1 == all.size());
                active.push_back(id);
            }
            else if (type == 2 && !active.empty())
            {
                const auto i = random() % active.size();
                lines.erase(active[i]);
                all[active[i]].active = false;
                active[i] = active.back();
                active.pop_back();
            }
            else
            {
                const auto x = number();
                std::optional<std::int64_t> best;
                for (const auto &entry : all)
                {
                    if (entry.active && entry.first <= x && x < entry.last)
                    {
                        const auto value = entry.line.slope * x + entry.line.intercept;
                        if (!best || value < *best)
                        {
                            best = value;
                        }
                    }
                }
                assert(lines.query(x) == expected.size());
                expected.push_back(best);
            }
            if (step % 50 == 0)
            {
                assert(lines.solve() == expected);
            }
        }
        assert(lines.solve() == expected);
    }
}

void check_immutable_values()
{
    struct Score
    {
        const int value;

        bool operator<(const Score &other) const
        {
            return value < other.value;
        }
    };
    static_assert(std::is_move_constructible_v<Score> && !std::is_move_assignable_v<Score>);
    struct ScoredLine
    {
        int slope, intercept;

        Score operator()(int x) const
        {
            return {slope * x + intercept};
        }
    };
    sirkit::OfflineLiChaoTree<int, ScoredLine> tree;
    tree.query(0);
    tree.add_line({2, 0});
    const auto second = tree.add_line({0, 3});
    tree.query(-2);
    tree.query(2);
    tree.erase(second);
    tree.query(2);
    const auto result = tree.solve();
    assert(result.size() == 4 && !result[0]);
    assert(result[1] && result[1]->value == -4);
    assert(result[2] && result[2]->value == 3);
    assert(result[3] && result[3]->value == 4);
}

int main()
{
    check_limits();
    check_independent();
    check_immutable_values();

    auto function = [slope = std::make_unique<std::int64_t>(3)](std::int64_t x)
    {
        return *slope * x;
    };
    using MoveOnlyTree = sirkit::OfflineLiChaoTree<std::int64_t, decltype(function)>;
    static_assert(!std::is_copy_constructible_v<MoveOnlyTree>);
    MoveOnlyTree tree;
    tree.add_line(std::move(function));
    tree.query(4);
    const MoveOnlyTree moved(std::move(tree));
    assert(moved.solve().front() == 12);
}
