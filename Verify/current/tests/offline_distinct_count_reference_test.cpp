// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_OFFLINE_DISTINCT_COUNT_TEST_SOURCE
#define SIRKIT_OFFLINE_DISTINCT_COUNT_TEST_SOURCE "../../../Algorithm/Offline_Distinct_Count.cpp"
#endif
#include SIRKIT_OFFLINE_DISTINCT_COUNT_TEST_SOURCE
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
using Query = sirkit::DistinctCountQuery;

std::vector<std::size_t> Direct(std::span<const std::int64_t> values, std::span<const Query> queries)
{
    std::vector<std::size_t> answer;
    for (const auto &[left, right] : queries)
    {
        auto slice = values.subspan(left, right - left);
        answer.push_back(std::set<std::int64_t>(slice.begin(), slice.end()).size());
    }
    return answer;
}

#ifndef SKIP_RUNTIME_CHECKS
bool Rejects(std::span<const std::int64_t> values, Query query)
{
    try
    {
        sirkit::offline_distinct_count(values, std::array{query});
    }
    catch (const std::out_of_range &)
    {
        return true;
    }
    return false;
}
#endif
} // namespace

int main()
{
    assert(sirkit::offline_distinct_count({}, {}).empty());
    assert((sirkit::offline_distinct_count({}, std::array{Query{0, 0}, Query{0, 0}}) ==
            std::vector<std::size_t>{0, 0}));

    const std::array distinct_extremes{
        std::numeric_limits<std::int64_t>::min(),
        std::int64_t{0},
        std::numeric_limits<std::int64_t>::max()};
    std::mt19937 random(275063);
    for (std::size_t n = 1, combinations = 3; n <= 7; ++n, combinations *= 3)
    {
        std::vector<Query> queries;
        for (std::size_t right = 0; right <= n; ++right)
        {
            for (std::size_t left = 0; left <= right; ++left)
            {
                queries.push_back({left, right});
            }
        }
        std::shuffle(queries.begin(), queries.end(), random);

        for (std::size_t p = 0; p < combinations; ++p)
        {
            std::vector<std::int64_t> values(n);
            auto digits = p;
            for (auto &value : values)
            {
                value = distinct_extremes[digits % distinct_extremes.size()];
                digits /= distinct_extremes.size();
            }
            assert(sirkit::offline_distinct_count(values, queries) == Direct(values, queries));
        }
    }

    // 多数の同じ右端を持ち、最後の出現だけが区間の内外を分ける。
    constexpr std::size_t kSize = 67'000;
    constexpr std::size_t kPeriod = 257;
    std::vector<std::int64_t> repeated(kSize);
    std::vector<Query> suffixes;
    for (std::size_t left = 0; left < kSize; ++left)
    {
        repeated[left] = static_cast<std::int64_t>(left % kPeriod);
        suffixes.push_back({left, kSize});
    }
    suffixes.push_back({kSize, kSize});
    std::shuffle(suffixes.begin(), suffixes.end(), random);
    const auto counts = sirkit::offline_distinct_count(repeated, suffixes);
    for (std::size_t i = 0; i < counts.size(); ++i)
    {
        assert(counts[i] == std::min(kPeriod, kSize - suffixes[i].left));
    }

#ifndef SKIP_RUNTIME_CHECKS
    assert(Rejects(distinct_extremes, {2, 1}));
    assert(Rejects(distinct_extremes, {0, 4}));
    assert(Rejects(distinct_extremes, {4, 4}));
    assert(Rejects({}, {1, 1}));
#endif
}
