// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_PREFIX_SUM_TEST_SOURCE
#define SIRKIT_PREFIX_SUM_TEST_SOURCE "../../../Algorithm/Prefix_Sum.cpp"
#endif
#include SIRKIT_PREFIX_SUM_TEST_SOURCE
#include <array>
#include <cassert>
#include <cstdint>
#include <numeric>

int main()
{
    using Sum = sirkit::PrefixSum<std::int64_t>;
    Sum empty;
    assert(empty.size() == 0 && empty.prefix(0) == 0 && empty.sum(0, 0) == 0);
    const std::array<std::int64_t, 5> values{3'000'000'000LL, -8, 7, 4'000'000'000LL, -4};
    const Sum sums(values);
    assert(sums.size() == values.size());
    for (std::size_t right = 0; right <= values.size(); ++right)
    {
        assert(sums.prefix(right) == std::accumulate(values.begin(), values.begin() + right, std::int64_t{0}));
        for (std::size_t left = 0; left <= right; ++left)
        {
            const auto expected = std::accumulate(values.begin() + left, values.begin() + right, std::int64_t{0});
            assert(sums.sum(left, right) == expected);
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    const auto out_of_range = [](auto operation)
    {
        try
        {
            operation();
        }
        catch (const std::out_of_range &)
        {
            return true;
        }
        return false;
    };
    assert(out_of_range([&]
                        {
                            sums.prefix(6);
                        }));
    assert(out_of_range([&]
                        {
                            sums.sum(2, 1);
                        }));
    assert(out_of_range([&]
                        {
                            sums.sum(0, 6);
                        }));
#endif
}
