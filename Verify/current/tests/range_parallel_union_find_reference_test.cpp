// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Range_Parallel_Union_Find.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

int main()
{
    using sirkit::RangeParallelUnionFind;

    RangeParallelUnionFind empty;
    assert(empty.empty() && empty.size() == 0 && empty.component_count() == 0);
    assert(empty.merge_range(0, 0, 0) == 0);
    RangeParallelUnionFind small(4);
    assert(!small.empty() && small.size() == 4);
    assert(!small.same(0, 1));
    small.unite(0, 1);
    assert(small.same(0, 1) && !small.merge(0, 1));
    assert(small.merge(0, 2) && small.leader(2) == small.root(0));
    assert(small.size(0) == 3);
    assert(small.merge(3, 0));
    assert(small.component_count() == 1);
    assert(small.merge_range(0, 1, 2) == 0);

    RangeParallelUnionFind before(7);
    before.merge_range(1, 3, 3);
    const auto copied = before;
    before.merge(0, 1);
    auto moved = std::move(before);
    assert(before.size() == 0 && before.component_count() == 0);
    assert(moved.same(0, 1) && moved.component_count() < copied.component_count());
    auto &alias = moved;
    moved = std::move(alias);
    assert(moved.issame(0, 3));
    before = std::move(moved);
    assert(moved.empty() && before.same(0, 3));

    RangeParallelUnionFind interrupted(16);
    int notified = 0;
    try
    {
        interrupted.merge_range(0, 8, 7, [&](int keep, int absorbed)
                                {
                                    assert(interrupted.root(keep) == interrupted.root(absorbed));
                                    if (++notified == 6)
                                    {
                                        throw std::runtime_error("callback");
                                    }
                                });
        assert(false);
    }
    catch (const std::runtime_error &)
    {
    }
    assert(notified == 6 && interrupted.component_count() == 10);
    assert(interrupted.merge_range(0, 8, 7) == 1);
    for (int i = 0; i < 7; ++i)
    {
        assert(interrupted.same(i, i + 8) && interrupted.size(i) == 2);
    }

    std::mt19937 rng(299311);
    for (int trial = 0; trial < 700; ++trial)
    {
        const int n = trial % 45 + 1;
        RangeParallelUnionFind candidate(n);
        std::vector<int> reference(n);
        std::iota(reference.begin(), reference.end(), 0);
        std::vector<std::int64_t> weights(n);
        for (auto &weight : weights)
        {
            weight = static_cast<std::int64_t>(rng() % 137) - 68;
        }
        auto sums = weights;
        int reported = 0;
        const auto on_merge = [&](int keep, int absorbed)
        {
            assert(keep != absorbed);
            assert(candidate.root(keep) == keep && candidate.root(absorbed) == keep);
            assert(candidate.size(keep) >= 2);
            ++reported;
            sums[keep] += sums[absorbed];
            sums[absorbed] = 0;
        };

        int component_count = n;
        for (int step = 0; step < 65; ++step)
        {
            const int length = static_cast<int>(rng() % (n + 1));
            const int a = static_cast<int>(rng() % (n - length + 1));
            const int b = static_cast<int>(rng() % (n - length + 1));
            int actual_merges = 0;
            for (int offset = 0; offset < length; ++offset)
            {
                const auto keep = reference[a + offset];
                const auto gone = reference[b + offset];
                if (keep != gone)
                {
                    ++actual_merges;
                    std::replace(reference.begin(), reference.end(), gone, keep);
                }
            }

            const int previous_reports = reported;
            assert(candidate.merge_range(a, b, length, on_merge) == actual_merges);
            assert(reported - previous_reports == actual_merges);
            component_count -= actual_merges;
            assert(candidate.component_count() == component_count);
            const int v = static_cast<int>(rng() % n);
            std::int64_t sum = 0;
            int count = 0;
            for (int i = 0; i < n; ++i)
            {
                assert(candidate.issame(v, i) == (reference[v] == reference[i]));
                if (reference[v] == reference[i])
                {
                    sum += weights[i];
                    ++count;
                }
            }
            assert(sums[candidate.root(v)] == sum && candidate.size(v) == count);
        }
    }

    try
    {
        const RangeParallelUnionFind too_big(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }

#ifndef SKIP_RUNTIME_CHECKS
    for (int i : {-1, 4})
    {
        try
        {
            (void)small.root(i);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }

    constexpr std::array bad_ranges{
        std::array{0, 0, -1},
        std::array{0, 0, 5},
        std::array{-1, 0, 1},
        std::array{0, -1, 1},
        std::array{4, 0, 1},
        std::array{0, 4, 1},
    };
    for (const auto &[a, b, length] : bad_ranges)
    {
        try
        {
            small.merge_range(a, b, length);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
