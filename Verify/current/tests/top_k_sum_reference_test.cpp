// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Top_K_Sum.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <vector>

template <bool Largest>
void Check()
{
    using Values = sirkit::TopKSum<int, Largest, std::int64_t>;
    Values values;
    std::vector<int> reference;
    std::size_t k = 0;
    std::mt19937 rng(984215);
    for (int index = 0; index < 5000; ++index)
    {
        const int value = static_cast<int>(rng() % 41) - 20;
        switch (rng() % 5)
        {
        case 0:
        case 1:
            values.insert(value);
            reference.push_back(value);
            break;
        case 2:
        {
            const auto found = std::find(reference.begin(), reference.end(), value);
            assert(values.erase(value) == (found != reference.end()));
            if (found != reference.end())
            {
                reference.erase(found);
            }
            break;
        }
        case 3:
            k = rng() % 75;
            values.set_k(k);
            break;
        default:
            if (rng() % 7 == 0)
            {
                values.clear();
                reference.clear();
            }
            break;
        }
        std::sort(reference.begin(), reference.end());
        if constexpr (Largest)
        {
            std::reverse(reference.begin(), reference.end());
        }
        const std::size_t count = std::min(k, reference.size());
        assert(values.size() == reference.size() && values.empty() == reference.empty() && values.k() == k);
        assert(values.sum() == std::accumulate(reference.begin(), reference.begin() + static_cast<std::ptrdiff_t>(count),
                                               std::int64_t{0}));
        const std::optional<int> kth = k == 0 || reference.size() < k ? std::nullopt : std::optional(reference[k - 1]);
        assert(values.kth() == kth);
    }
    Values copied = values;
    Values moved = std::move(values);
    assert(values.empty() && values.k() == 0 && values.sum() == 0);
    values = std::move(copied);
    assert(values.sum() == moved.sum() && values.k() == k && values.kth() == moved.kth());
}

int main()
{
    Check<false>();
    Check<true>();
    sirkit::TopKSum<std::int64_t> values(2);
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    values.insert(kMax);
    values.insert(0);
    values.insert(kMin);
    assert(values.sum() == kMin && values.kth() == 0);
    assert(values.erase(0) && values.sum() == -1 && values.kth() == kMax);
    values.set_k(std::numeric_limits<std::size_t>::max());
    assert(values.sum() == -1 && !values.kth());
    values.set_k(0);
    assert(values.sum() == 0 && !values.kth());
}
