// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Static_Range_Mode.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
template <class T>
void CheckAllRanges(const std::vector<T> &values)
{
    const sirkit::StaticRangeMode<T> mode(values);
    const auto n = static_cast<int>(values.size());
    assert(mode.size() == n);
    for (int l = 0; l <= n; ++l)
    {
        assert(!mode.query(l, l));
        std::map<T, int> counts;
        int best = 0;
        for (int r = l + 1; r <= n; ++r)
        {
            best = std::max(best, ++counts[values[r - 1]]);
            const auto result = mode.query(l, r);
            assert(result && result->second == best && counts[result->first] == best);
        }
    }
}
}

int main()
{
    using sirkit::StaticRangeMode;
    CheckAllRanges(std::vector<int>{});
    CheckAllRanges(std::vector<int>{7});
    CheckAllRanges(std::vector<std::int64_t>{
        std::numeric_limits<std::int64_t>::min(),
        5,
        std::numeric_limits<std::int64_t>::max(),
        5,
        -1,
        -1,
    });
    CheckAllRanges(std::vector<std::string>{"charlie", "alfa", "bravo", "bravo", "alfa"});
    CheckAllRanges(std::vector<int>(100, 42));
    std::vector<int> distinct(100);
    std::iota(distinct.begin(), distinct.end(), -50);
    CheckAllRanges(distinct);

    // n=100ではブロック長11であり、[11,88)が照会の中央になる。
    std::vector<int> fringe(100);
    std::iota(fringe.begin(), fringe.end(), 1000);
    std::fill(fringe.begin() + 11, fringe.begin() + 21, 1);
    std::fill(fringe.begin() + 22, fringe.begin() + 31, 2);
    fringe[10] = fringe[88] = fringe[89] = 2;
    const StaticRangeMode<int> fringe_mode(fringe);
    assert((fringe_mode.query(11, 88) == std::pair{1, 10}));
    assert((fringe_mode.query(10, 89) == std::pair{2, 11}));
    assert((fringe_mode.query(11, 90) == std::pair{2, 11}));
    const auto tied = fringe_mode.query(10, 88);
    assert(tied && (tied->first == 1 || tied->first == 2) && tied->second == 10);
    CheckAllRanges(fringe);

    std::mt19937 rng(7336798);
    for (int trial = 0; trial < 650; ++trial)
    {
        const int n = trial % 92;
        std::vector<int> values(n);
        for (auto &value : values)
        {
            value = static_cast<int>(rng() % (1 + trial % 71)) - 37;
        }
        CheckAllRanges(values);
    }

    sirkit::detail::CheckStaticRangeModeSize(std::numeric_limits<int>::max());
    try
    {
        sirkit::detail::CheckStaticRangeModeSize(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }

#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[left, right] : {std::pair{-1, 4}, std::pair{10, 9}, std::pair{0, 101}})
    {
        try
        {
            (void)fringe_mode.query(left, right);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
