// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Affine_Walk_Ranges.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::AffineWalkEdge;
using Range = sirkit::WalkWeightRange;

std::vector<Range> oracle(int n, const std::vector<Edge> &edges, int start)
{
    int longest = 0;
    std::vector<std::vector<Edge>> adjacency(n);
    for (const auto &edge : edges)
    {
        longest = std::max(longest, std::abs(edge.shift));
        adjacency[edge.from].push_back(edge);
    }
    const int bound = 2 * n * longest;
    std::vector<std::vector<bool>> seen(n, std::vector<bool>(2 * bound + 1));
    seen[start][bound] = true;
    std::vector<std::pair<int, int>> pending{{start, 0}};
    std::vector<Range> answer(n);
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        const auto [here, current] = pending[i];
        auto &found = answer[here];
        if (!found.reachable)
        {
            found = {true, found.unbounded, current, current};
        }
        else
        {
            found.minimum = std::min(found.minimum, static_cast<std::int64_t>(current));
            found.maximum = std::max(found.maximum, static_cast<std::int64_t>(current));
        }
        for (const auto &[u, v, reflect, shift] : adjacency[here])
        {
            const int next = (reflect ? -current : current) + shift;
            if (std::abs(next) > bound)
            {
                answer[v].unbounded = true;
            }
            else if (!seen[v][next + bound])
            {
                seen[v][next + bound] = true;
                pending.emplace_back(v, next);
            }
        }
    }
    std::vector<int> unbounded;
    for (int v = 0; v < n; ++v)
    {
        if (answer[v].unbounded)
        {
            unbounded.push_back(v);
        }
    }
    for (std::size_t i = 0; i < unbounded.size(); ++i)
    {
        const int v = unbounded[i];
        answer[v] = {true, true, 0, 0};
        for (const auto &edge : adjacency[v])
        {
            if (!answer[edge.to].unbounded)
            {
                answer[edge.to].unbounded = true;
                unbounded.push_back(edge.to);
            }
        }
    }
    return answer;
}

void check(int n, const std::vector<Edge> &edges, int start)
{
    const auto actual = sirkit::affine_walk_ranges(n, edges, start);
    const auto expected = oracle(n, edges, start);
    assert(actual.size() == expected.size());
    for (int v = 0; v < n; ++v)
    {
        assert(actual[v].reachable == expected[v].reachable);
        assert(actual[v].unbounded == expected[v].unbounded);
        assert(actual[v].minimum == expected[v].minimum);
        assert(actual[v].maximum == expected[v].maximum);
    }
}

template <class Error, class Action>
void expect_error(Action action)
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
    check(1, {}, 0);
    check(1, {{0, 0, true, 3}}, 0);
    check(1, {{0, 0, true, 1}, {0, 0, true, -1}}, 0);
    check(4, {{0, 1, true, 3}, {0, 1, false, -4}, {1, 2, true, -1}}, 0);
    check(3, {{0, 1, true, -1}, {1, 1, false, -1}, {1, 2, false, 2}}, 0);
    check(4, {{0, 1, false, 2}, {1, 1, false, -1}, {1, 2, true, 2}}, 0);
    check(5, {{0, 1, false, 0}, {1, 2, false, 2}, {2, 1, false, -2}, {2, 3, true, 1}, {3, 4, false, -1}}, 0);

    constexpr auto kMaximumShift = std::numeric_limits<std::int32_t>::max();
    const std::array extreme{Edge{0, 1, false, kMaximumShift}, Edge{1, 2, false, kMaximumShift}, Edge{2, 3, true, -kMaximumShift}};
    const auto exact = sirkit::affine_walk_ranges(4, extreme, 0);
    assert(exact[2].minimum == 2LL * kMaximumShift && exact[2].maximum == exact[2].minimum);
    assert(exact[3].minimum == -3LL * kMaximumShift && exact[3].maximum == exact[3].minimum);

    for (const int n : {0, -1, std::numeric_limits<int>::max()})
    {
        expect_error<std::length_error>([&]
                                        {
                                            (void)sirkit::affine_walk_ranges(n, {}, 0);
                                        });
    }
    for (const int source : {-1, 2})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::affine_walk_ranges(2, {}, source);
                                        });
    }
    for (const auto &edge : {Edge{-1, 0, true, 0}, Edge{2, 0, false, 0}, Edge{0, -1, true, 0}, Edge{0, 2, false, 0},
                             Edge{0, 1, false, std::numeric_limits<std::int32_t>::min()}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array invalid{edge};
                                            (void)sirkit::affine_walk_ranges(2, invalid, 0);
                                        });
    }

    std::mt19937 random(83723112);
    for (int trial = 0; trial < 1200; ++trial)
    {
        const int n = static_cast<int>(random() % 11) + 1;
        std::vector<Edge> edges(random() % 20);
        for (auto &edge : edges)
        {
            edge = {static_cast<int>(random() % n), static_cast<int>(random() % n), (random() % 2) != 0,
                    static_cast<int>(random() % 9) - 4};
        }
        check(n, edges, static_cast<int>(random() % n));
    }
}
