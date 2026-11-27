// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Range_Inversions.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

int main()
{
    try
    {
        sirkit::detail::check_range_inversion_size(std::uint64_t{1} << 32);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    std::mt19937 random(481);
    for (std::size_t n = 0; n < 110; ++n)
    {
        std::vector<int> values(n);
        for (auto &x : values)
        {
            x = static_cast<int>(random() % 19) - 9;
        }
        std::vector<std::pair<std::size_t, std::size_t>> queries;
        for (int q = 0; q < 500; ++q)
        {
            auto l = random() % (n + 1), r = random() % (n + 1);
            if (l > r)
            {
                std::swap(l, r);
            }
            queries.emplace_back(l, r);
        }
        queries.emplace_back(0, n);
        const auto result = sirkit::range_inversion_counts(values, queries);
        for (std::size_t q = 0; q < result.size(); ++q)
        {
            const auto [l, r] = queries[q];
            std::uint64_t count = 0;
            for (auto i = l; i < r; ++i)
            {
                for (auto j = i + 1; j < r; ++j)
                {
                    count += values[i] > values[j];
                }
            }
            assert(result[q] == count);
        }
        assert(sirkit::inversion_count(values) == result.back());
        assert(sirkit::range_inversion_counts(values, {}).empty());
    }
#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<int> values{4};
    for (const auto &query : {std::pair<std::size_t, std::size_t>{1, 0}, {0, 2}})
    {
        try
        {
            sirkit::range_inversion_counts(values, {&query, 1});
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
