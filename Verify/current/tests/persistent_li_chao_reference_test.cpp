// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Persistent_Li_Chao_Tree.cpp"
#include <cassert>
#include <limits>
#include <random>
#include <string_view>

namespace
{
using Tree = sirkit::PersistentLiChaoTree<std::int64_t>;
using Line = Tree::Line;
using Version = Tree::Version;

void test_boundaries()
{
    Tree empty({});
    assert(!empty.get(empty.add_line({}, {1, 3}), 0));
    Tree tree({6, -10, 0, 0, 20});
    const auto full = tree.add_line({}, {0, std::numeric_limits<std::int64_t>::max()});
    assert(tree.get(full, 20) == std::numeric_limits<std::int64_t>::max());
    const auto partial = tree.add_segment({}, -11, 6, {1, 10});
    assert(tree.get(partial, -10) == 0);
    assert(tree.get(partial, 0) == 10);
    assert(!tree.get(partial, 6));
    assert(!tree.get(partial, 21) && !tree.get(partial, 2));
    const auto nonoverlapping = tree.add_segment(partial, 1, 5, {0, -1});
    assert(tree.get(nonoverlapping, 0) == 10);
    const auto same = tree.add_segment(partial, -11, 6, {1, 10});
    assert(tree.get(same, 0) == 10);
    const auto dominant = tree.add_line({}, {0, -1});
    const auto replace = tree.add_line(dominant, {0, -2});
    assert(tree.get(replace, -10) == -2);
    assert(tree.get(dominant, -10) == -1);
    Tree assigned({100});
    assigned = tree;
    assert(assigned.get(assigned.add_line({}, {0, 3}), -10) == 3);
    assigned = Tree({50});
    assert(assigned.get(assigned.add_line({}, {0, 4}), 50) == 4);
}

void test_random()
{
    struct BoundedLine
    {
        std::int64_t from;
        std::int64_t to;
        Line line;
    };
    std::mt19937 random(93018);
    for (int trial = 0; trial < 36; ++trial)
    {
        std::vector<std::int64_t> xs;
        for (int i = 0; i < 28; ++i)
        {
            xs.push_back(static_cast<int>(random() % 250) - 125);
        }
        Tree tree(xs);
        std::vector<Version> versions(1);
        std::vector<std::vector<BoundedLine>> model(1);
        for (int step = 0; step < 220; ++step)
        {
            const auto older = random() % versions.size();
            const Line line{static_cast<int>(random() % 73) - 36, static_cast<int>(random() % 81) - 40};
            auto copy = model[older];
            std::int64_t low = static_cast<int>(random() % 260) - 130;
            std::int64_t high = static_cast<int>(random() % 260) - 130;
            if (high < low)
            {
                std::swap(high, low);
            }
            if (random() % 3 == 0)
            {
                versions.push_back(tree.add_line(versions[older], line));
                low = -1000;
                high = 1000;
            }
            else
            {
                versions.push_back(tree.add_segment(versions[older], low, high, line));
            }
            copy.push_back({low, high, line});
            model.push_back(std::move(copy));
            for (const auto index : {std::size_t(older), versions.size() - 1})
            {
                for (const auto x : xs)
                {
                    std::optional<std::int64_t> expected;
                    for (const auto &[l, r, f] : model[index])
                    {
                        if (l <= x && x < r)
                        {
                            const auto y = x * f.slope + f.intercept;
                            if (!expected || y < *expected)
                            {
                                expected = y;
                            }
                        }
                    }
                    assert(tree.get(versions[index], x) == expected);
                }
            }
        }
    }
}

#ifndef SKIP_RUNTIME_CHECKS
template <class Error, class Function>
void expect_failure(Function fn)
{
    bool rejected = false;
    try
    {
        fn();
    }
    catch (const Error &)
    {
        rejected = true;
    }
    assert(rejected);
}
#endif
} // namespace

int main(int argc, char **argv)
{
#ifndef SKIP_RUNTIME_CHECKS
    Tree tree({1, 2});
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        tree.add_segment({}, 2, 1, {0, 0});
        return 1;
    }
    expect_failure<std::invalid_argument>([&]
                                          {
                                              tree.add_segment({}, 2, 1, {0, 0});
                                          });
    const auto v = tree.add_line({}, {0, 1});
    const Tree other({1, 2});
    expect_failure<std::out_of_range>([&]
                                      {
                                          other.get(v, 1);
                                      });
#endif
    static_cast<void>(argc);
    static_cast<void>(argv);
    test_boundaries();
    test_random();
}
