// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Range_Add_Weighted_Minimum.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

int main()
{
    sirkit::RangeAddWeightedMinimum<> empty(std::vector<std::uint64_t>{});
    assert(empty.size() == 0 && !empty.all_minimum() && !empty.minimum(0, 0));
    const std::vector<std::uint64_t> base_weights{2, 3, 7};
    sirkit::RangeAddWeightedMinimum<> zeroes(base_weights);
    assert(zeroes.all_minimum()->value == 0 && zeroes.all_minimum()->weight == 12);
    zeroes.add(0, 2, 4);
    assert(zeroes.all_minimum()->value == 0 && zeroes.all_minimum()->weight == 7);
    const std::vector<int> values{6, -2, -2, 5};
    const std::vector<std::int64_t> weights{2, -3, 3, 7};
    sirkit::RangeAddWeightedMinimum<int, std::int64_t> sample(values, weights);
    assert(sample.all_minimum()->value == -2 && sample.all_minimum()->weight == 0);
    sample.add(1, 3, 10);
    assert(sample.minimum(0, 3)->value == 6);
    assert(sample.all_minimum()->value == 5 && sample.all_minimum()->weight == 7);
    bool caught = false;
    try
    {
        sirkit::RangeAddWeightedMinimum<int, std::int64_t> mismatch(values, {});
        (void)mismatch;
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught);

    std::mt19937 random(521119);
    for (int trial = 0; trial < 440; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 55);
        std::vector<int> values(n);
        std::vector<std::int64_t> weights(n);
        for (int i = 0; i < n; ++i)
        {
            values[i] = static_cast<int>(random() % 31) - 15;
            weights[i] = static_cast<int>(random() % 19) - 9;
        }
        sirkit::RangeAddWeightedMinimum<int, std::int64_t> data(values, weights);
        assert(data.size() == n);
        const auto check = [&](int begin, int end, auto result)
        {
            if (begin == end)
            {
                assert(!result);
                return;
            }
            const auto best = *std::min_element(values.begin() + begin, values.begin() + end);
            std::int64_t sum = 0;
            for (int i = begin; i < end; ++i)
            {
                if (values[i] == best)
                {
                    sum += weights[i];
                }
            }
            assert(result->value == best && result->weight == sum);
        };
        const auto index = [&]
        {
            return static_cast<int>(random() % (n + 1));
        };
        for (int step = 0; step < 100; ++step)
        {
            const auto [l, r] = std::minmax({index(), index()});
            const int delta = static_cast<int>(random() % 21) - 10;
            data.add(l, r, delta);
            for (int i = l; i < r; ++i)
            {
                values[i] += delta;
            }
            const auto [begin, end] = std::minmax({index(), index()});
            check(begin, end, data.minimum(begin, end));
            check(0, n, data.all_minimum());
        }
    }
}
