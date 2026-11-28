// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Eytzinger_Search.cpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

int main()
{
    std::mt19937 random(183447);
    for (int n = 0; n < 543; ++n)
    {
        std::vector<int> values(n);
        for (auto &value : values)
        {
            value = static_cast<int>(random() % 211) - 105;
        }
        const sirkit::EytzingerSearch data(values);
        std::ranges::sort(values);
        const auto ordered = sirkit::EytzingerSearch<int>::from_sorted(values);
        assert(data.size() == values.size());
        for (int key = -106; key <= 106; ++key)
        {
            const auto [left, right] = std::ranges::equal_range(values, key);
            const auto a = static_cast<std::size_t>(left - values.begin());
            const auto b = static_cast<std::size_t>(right - values.begin());
            assert(data.lower_bound(key) == a && ordered.lower_bound(key) == a);
            assert(data.upper_bound(key) == b && ordered.upper_bound(key) == b);
            assert(data.equal_range(key) == std::pair(a, b));
        }
    }

    const sirkit::EytzingerSearch decreasing(std::vector{2, 7, 8, 2, 3, 1}, std::greater<>{});
    assert(decreasing.lower_bound(2LL) == 3 && decreasing.upper_bound(2LL) == 5);
    const sirkit::EytzingerSearch words(std::vector<std::string>{"pq", "a", "a", "ac", ""});
    const auto [left, right] = words.equal_range(std::string_view("a"));
    assert(left == 1 && right == 3);
    const sirkit::EytzingerSearch bits(std::vector<bool>{true, false, true, false, false});
    assert(bits.lower_bound(true) == 3 && bits.upper_bound(true) == 5);

    bool caught = false;
    try
    {
        (void)sirkit::EytzingerSearch<int>::from_sorted({2, 0, 1});
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught);
}
