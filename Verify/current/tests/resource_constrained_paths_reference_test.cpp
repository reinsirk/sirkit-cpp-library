// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Resource_Constrained_Paths.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Arc = sirkit::ResourcePathArc;
using Paths = sirkit::ResourceConstrainedPaths;

// Bellman–Ford on all (vertex, exact resource) states.
void check_small(int n, const std::vector<Arc> &edges, int source, int capacity, int initial)
{
    const Paths actual(n, edges, source, capacity, initial);
    const int stride = capacity + 1;
    const int states = n * stride;
    constexpr std::uint64_t kInf = 1'000'000;
    std::vector<std::uint64_t> distance(states, kInf);
    distance[source * stride + std::min(initial, capacity)] = 0;
    for (int step = 1; step < states; ++step)
    {
        bool changed = false;
        for (const auto &edge : edges)
        {
            for (std::uint64_t before = edge.spend; before <= static_cast<std::uint64_t>(capacity); ++before)
            {
                const auto after = std::min(before - edge.spend + edge.gain, static_cast<std::uint64_t>(capacity));
                const auto next = edge.to * stride + after;
                const auto candidate = distance[edge.from * stride + before] + edge.time;
                if (candidate < distance[next])
                {
                    distance[next] = candidate;
                    changed = true;
                }
            }
        }
        if (!changed)
        {
            break;
        }
    }

    for (int v = 0; v < n; ++v)
    {
        const auto options = actual.labels(v);
        auto next = options.rbegin();
        std::uint64_t best = kInf;
        assert(!actual.minimum_time(v, stride));
        for (int remaining = capacity; remaining >= 0; --remaining)
        {
            const auto time = distance[v * stride + remaining];
            if (time < best)
            {
                assert(next != options.rend());
                assert(next->remaining == static_cast<std::uint64_t>(remaining) && next->time == time);
                ++next;
                best = time;
            }
            const auto minimum = actual.minimum_time(v, remaining);
            assert(minimum == (best < kInf ? std::optional(best) : std::nullopt));
        }
        assert(next == options.rend());
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
    check_small(1, {}, 0, 0, 0);
    check_small(3, {}, 1, 5, 23);
    // Better time at the same state, later replacement by a fuller state, and an equal-time tie.
    check_small(5, {{0, 1, 1, 0, 9}, {0, 2, 2, 0, 6}, {0, 3, 0, 0, 1}, {3, 1, 1, 0, 1}, {3, 2, 0, 0, 2}, {3, 3, 2, 0, 0}, {3, 4, 4, 0, 0}, {2, 4, 4, 1, 1}, {1, 4, 0, 0, 2}, {4, 4, 0, 4, 0}, {4, 0, 6, 0, 0}},
                0, 5, 4);
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const std::vector<Arc> wide{{0, 1, 1, kMax, kMax}, {1, 2, 0, 0, 1}, {1, 3, kMax, 0, 0}};
    const Paths paths(4, wide, 0, kMax, kMax - 1);
    assert(paths.minimum_time(0, kMax - 1) == 0 && !paths.minimum_time(0, kMax));
    assert(paths.minimum_time(1, kMax) == kMax && paths.labels(1).size() == 1);
    assert(!paths.minimum_time(2) && paths.minimum_time(3) == kMax && !paths.minimum_time(3, 1));

    std::mt19937 random(1649025);
    for (int trial = 0; trial < 920; ++trial)
    {
        const int n = static_cast<int>(random() % 7) + 1;
        std::vector<Arc> edges;
        const auto m = random() % (n * n * 2 + 1);
        for (unsigned i = 0; i < m; ++i)
        {
            edges.push_back({static_cast<int>(random() % n), static_cast<int>(random() % n),
                             random() % 11, random() % 11, random() % 13});
        }
        check_small(n, edges, static_cast<int>(random() % n), static_cast<int>(random() % 11),
                    static_cast<int>(random() % 21));
    }

    check_error<std::length_error>([]
                                   {
                                       Paths bad(-1, {}, 0, 0, 0);
                                   });
#ifndef SKIP_RUNTIME_CHECKS
    const Paths unit(1, {}, 0, 0, 0);
    for (int id : {-1, 1})
    {
        check_error<std::out_of_range>([&]
                                       {
                                           Paths bad(1, {}, id, 0, 0);
                                       });
        check_error<std::out_of_range>([&]
                                       {
                                           (void)unit.labels(id);
                                       });
        const std::vector<Arc> from{{id, 0, 0, 0, 0}}, to{{0, id, 0, 0, 0}};
        check_error<std::out_of_range>([&]
                                       {
                                           Paths bad(1, from, 0, 0, 0);
                                       });
        check_error<std::out_of_range>([&]
                                       {
                                           Paths bad(1, to, 0, 0, 0);
                                       });
    }
#endif
}
