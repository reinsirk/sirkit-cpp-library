// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_RANGE_MEX_DEFINED
#include "../../../Algorithm/Range_Mex.cpp"
#endif
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
std::size_t brute_mex(std::span<const std::int64_t> values)
{
    std::vector<bool> seen(values.size() + 1);
    for (const auto value : values)
    {
        if (value >= 0 && static_cast<std::uint64_t>(value) < seen.size())
        {
            seen[static_cast<std::size_t>(value)] = true;
        }
    }
    return static_cast<std::size_t>(std::find(seen.begin(), seen.end(), false) - seen.begin());
}

void check(std::span<const std::int64_t> values, std::mt19937 &random)
{
    std::vector<sirkit::RangeMexQuery> queries;
    std::vector<std::uint64_t> histogram(values.size() + 1);
    for (std::size_t left = 0; left <= values.size(); ++left)
    {
        for (std::size_t right = left; right <= values.size(); ++right)
        {
            queries.push_back({left, right});
            if (left != right)
            {
                ++histogram[brute_mex(values.subspan(left, right - left))];
            }
        }
    }
    assert(sirkit::subarray_mex_histogram(values) == histogram);
    std::shuffle(queries.begin(), queries.end(), random);
    queries.push_back(queries.front());
    const auto actual = sirkit::range_mex(values, queries);
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        const auto [left, right] = queries[i];
        assert(actual[i] == brute_mex(values.subspan(left, right - left)));
    }
}

template<class Exception, class Function>
void check_error(Function run)
{
    auto thrown = false;
    try
    {
        run();
    }
    catch (const Exception &)
    {
        thrown = true;
    }
    assert(thrown);
}
} // namespace

int main()
{
    using Value = std::int64_t;
    std::mt19937 random(15345376);
    check({}, random);
    check(std::array<Value, 7>{-1, 1, -1, 1, -1, 0, 1}, random);
    check(std::array<Value, 6>{3, 2, 1, 0, 6, std::numeric_limits<Value>::max()}, random);
    check(std::array<Value, 6>{0, 1, 2, 3, std::numeric_limits<Value>::min(), -1}, random);
    assert(sirkit::range_mex(std::array<Value, 2>{1, 0}, {}).empty());
    for (int trial = 0; trial < 900; ++trial)
    {
        std::vector<Value> values(random() % 24);
        const auto alphabet = random() % 10 + 1;
        for (auto &value : values)
        {
            value = static_cast<Value>(random() % (alphabet + 2)) - 2;
        }
        check(values, random);
    }

    constexpr std::size_t kLength = 200'000;
    std::vector<Value> values(kLength);
    std::iota(values.begin(), values.end(), 0);
    std::vector<sirkit::RangeMexQuery> queries(kLength);
    for (std::size_t i = 0; i < kLength; ++i)
    {
        queries[i] = {i % 2, i + 1};
    }
    const auto mex = sirkit::range_mex(values, queries);
    const auto histogram = sirkit::subarray_mex_histogram(values);
    assert(histogram[0] == std::uint64_t{kLength} * (kLength - 1) / 2);
    for (std::size_t i = 0; i < kLength; ++i)
    {
        assert(mex[i] == (i % 2 == 0 ? i + 1 : 0));
        assert(histogram[i + 1] == 1);
    }

#ifndef SKIP_RUNTIME_CHECKS
    for (const auto query : {sirkit::RangeMexQuery{1, 0}, sirkit::RangeMexQuery{0, 1}})
    {
        check_error<std::out_of_range>([&] { (void)sirkit::range_mex({}, std::span(&query, 1)); });
    }
#endif
    check_error<std::length_error>([] {
        (void)sirkit::detail::range_mex_base(std::numeric_limits<std::size_t>::max());
    });
    if constexpr (sizeof(std::size_t) >= sizeof(std::uint64_t))
    {
        check_error<std::overflow_error>([] {
            (void)sirkit::detail::mex_subarray_count(std::numeric_limits<std::size_t>::max());
        });
    }
}
