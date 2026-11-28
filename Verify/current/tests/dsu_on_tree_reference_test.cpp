// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/DSU_On_Tree.cpp"

#include <algorithm>
#include <cassert>
#include <bit>
#include <cstddef>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
void check(const std::vector<int> &parents)
{
    const int n = static_cast<int>(parents.size());
    const sirkit::SubtreeIntervals intervals(parents);
    const sirkit::DsuOnTree tree(intervals);
    assert(tree.size() == n);
    std::vector<std::vector<bool>> expected(n, std::vector<bool>(n));
    for (int child = 0; child < n; ++child)
    {
        for (int v = child; v != -1; v = parents[v])
        {
            expected[v][child] = true;
        }
    }
    for (int repetitions = 0; repetitions < 2; ++repetitions)
    {
        std::vector<bool> live(n);
        std::vector<int> answers(n);
        auto add = [&, moves = std::make_unique<int>(0)](int v)
        {
            assert(!live[v]);
            live[v] = true;
            ++*moves;
        };
        tree.run(
            std::move(add),
            [&](int v)
            {
                assert(live[v]);
                live[v] = false;
            },
            [&](int v)
            {
                assert(live == expected[v]);
                ++answers[v];
            });
        assert(std::none_of(live.begin(), live.end(), [](bool bit) { return bit; }));
        assert(std::all_of(answers.begin(), answers.end(), [](int count) { return count == 1; }));
    }
}

template <typename Error, typename Action>
void check_error(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Error &)
    {
        caught = true;
    }
    assert(caught);
}
} // namespace

int main()
{
    check({});
    check({-1});
    check({3, 3, 4, -1, 0, 2, -1, 3, 7});
    check({-1, 0, 0, 0, 3, 3, 4, 2, 1});
    check({2, 2, -1, -1, 3, -1});

    std::mt19937 random(9131179);
    for (int test = 0; test < 950; ++test)
    {
        const int n = 1 + static_cast<int>(random() % 65);
        std::vector<int> parents(n, -1), order(n);
        std::iota(order.begin(), order.end(), 0);
        std::shuffle(order.begin(), order.end(), random);
        for (int i = 1; i < n; ++i)
        {
            if (random() % 6)
            {
                parents[order[i]] = order[random() % i];
            }
        }
        check(parents);
    }

    constexpr int kLength = 190000;
    std::vector<int> parents(kLength);
    std::iota(parents.begin(), parents.end(), -1);
    const sirkit::DsuOnTree chain(parents);
    int active = 0;
    std::size_t adds = 0, removes = 0;
    chain.run(
        [&](int) { ++active; ++adds; },
        [&](int) { --active; ++removes; },
        [&](int v) { assert(active == kLength - v); });
    assert(adds == kLength && removes == kLength && active == 0);

    constexpr int kBalanced = (1 << 14) - 1;
    parents.resize(kBalanced);
    for (int v = 1; v < kBalanced; ++v)
    {
        parents[v] = (v - 1) / 2;
    }
    const sirkit::DsuOnTree balanced(parents);
    adds = 0;
    removes = 0;
    balanced.run(
        [&](int) { ++active; ++adds; },
        [&](int) { --active; ++removes; },
        [&](int v)
        {
            const int level = static_cast<int>(std::bit_width(static_cast<unsigned>(v + 1)));
            assert(active == (1 << (15 - level)) - 1);
        });
    assert(adds == removes && adds <= 14 * kBalanced && active == 0);

    check_error<std::out_of_range>([] { const sirkit::DsuOnTree bad(std::vector<int>{-1, 3}); });
    check_error<std::invalid_argument>([] { const sirkit::DsuOnTree bad(std::vector<int>{1, 0}); });
}
