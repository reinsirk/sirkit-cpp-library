// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Subset_Sum.cpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <random>
#include <vector>

int main()
{
    std::mt19937 random(6176);
    for (std::size_t n = 0; n <= 12; ++n)
    {
        for (int trial = 0; trial < 60; ++trial)
        {
            std::vector<std::size_t> values(n);
            for (auto &value : values)
            {
                value = random() % 136;
            }
            const auto target = random() % 400;
            bool expected = false;
            for (std::size_t mask = 0; mask != (std::size_t{1} << n); ++mask)
            {
                std::size_t sum = 0;
                for (std::size_t bit = 0; bit < n; ++bit)
                {
                    if ((mask >> bit) & 1)
                    {
                        sum += values[bit];
                    }
                }
                expected |= sum == target;
            }
            const auto result = sirkit::subset_sum(values, target);
            assert(bool(result) == expected);
            if (result)
            {
                assert(std::ranges::is_sorted(*result));
                assert(std::ranges::adjacent_find(*result) == result->end());
                std::size_t sum = 0;
                for (auto i : *result)
                {
                    assert(i < n);
                    sum += values[i];
                }
                assert(sum == target);
            }
        }
    }
    const std::vector<std::size_t> words{0, 63, 64, 65, 128, 128, 4096};
    for (auto target : {0, 63, 64, 65, 127, 128, 129, 255, 256, 257, 4096, 4288})
    {
        const auto result = sirkit::subset_sum(words, target);
        assert(result);
        std::size_t sum = 0;
        for (auto index : *result)
        {
            sum += words[index];
        }
        assert(sum == static_cast<std::size_t>(target));
    }
    assert(!sirkit::subset_sum(words, 5000));
    const auto enormous = std::numeric_limits<std::size_t>::max();
    try
    {
        sirkit::subset_sum({&enormous, 1}, enormous);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
}
