// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Interval_Set.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>

int main()
{
    constexpr std::size_t kSize = 64;
    sirkit::IntervalSet<int> set;
    std::array<bool, kSize> present{};
    std::mt19937 rng(413238);
    assert(set.empty() && set.interval_count() == 0 && set.mex() == 0);
    for (int query = 0; query < 2500; ++query)
    {
        int left = static_cast<int>(rng() % (kSize + 1));
        int right = static_cast<int>(rng() % (kSize + 1));
        if (right < left)
        {
            std::swap(left, right);
        }
        const bool insert = (rng() % 3 == 0);
        if (insert)
        {
            set.add(left - 32, right - 32);
        }
        else
        {
            set.erase(left - 32, right - 32);
        }
        std::fill(present.begin() + left, present.begin() + right, insert);
        assert(set.size() == static_cast<unsigned>(std::count(present.begin(), present.end(), true)));
        assert(set.covers(left - 32, right - 32) == std::all_of(present.begin() + left, present.begin() + right,
                                                                [](bool value)
                                                                {
                                                                    return value;
                                                                }));
        std::size_t intervals = 0;
        for (std::size_t index = 0; index < kSize; ++index)
        {
            const int value = static_cast<int>(index) - 32;
            const auto end = std::find(present.begin() + static_cast<std::ptrdiff_t>(index), present.end(), false);
            const int expected_mex = static_cast<int>(end - present.begin()) - 32;
            assert(set.contains(value) == present[index]);
            assert(set.mex(value) == expected_mex);
            const auto interval = set.interval_at(value);
            assert(interval.has_value() == present[index]);
            if (interval)
            {
                std::size_t begin = index;
                while (begin > 0 && present[begin - 1])
                {
                    --begin;
                }
                assert(interval->first == static_cast<int>(begin) - 32);
                assert(interval->second == expected_mex);
                intervals += begin == index;
            }
        }
        assert(set.interval_count() == intervals);
    }

    sirkit::IntervalSet<std::int64_t> wide;
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    constexpr auto kWidth = std::numeric_limits<std::uint64_t>::max();
    wide.add(kMin, kMax);
    assert(wide.size() == kWidth && wide.mex(kMin) == kMax && !wide.contains(kMax));
    wide.erase(-1, 1);
    assert(wide.size() == kWidth - 2 && wide.interval_count() == 2 && wide.mex(0) == 0);
    assert(!wide.covers(kMin, 1));
    wide.add(-1, 1);
    assert(wide.size() == kWidth && wide.covers(kMin, kMax));
    wide.erase(kMin, 0);
    assert(wide.interval_count() == 1 && wide.interval_at(0)->first == 0);
    sirkit::IntervalSet<std::int64_t> moved(std::move(wide));
    assert(wide.empty() && wide.size() == 0 && moved.contains(0));
    wide = moved;
    moved.clear();
    assert(moved.empty() && moved.size() == 0 && wide.contains(0));
#ifndef SKIP_RUNTIME_CHECKS
    for (int operation = 0; operation < 3; ++operation)
    {
        bool thrown = false;
        try
        {
            if (operation == 0)
            {
                wide.add(7, 2);
            }
            if (operation == 1)
            {
                wide.erase(7, 2);
            }
            if (operation == 2)
            {
                (void)wide.covers(7, 2);
            }
        }
        catch (const std::invalid_argument &)
        {
            thrown = true;
        }
        assert(thrown && wide.covers(0, kMax) && wide.interval_count() == 1);
    }
#endif
}
