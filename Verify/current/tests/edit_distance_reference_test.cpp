// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Edit_Distance.cpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <random>
#include <string>
#include <vector>

int main()
{
    std::mt19937 random(59178);
    const auto make = [&](std::size_t n, unsigned alphabet)
    {
        std::string result(n, 'a');
        for (auto &letter : result)
        {
            letter += random() % alphabet;
        }
        return result;
    };
    for (int trial = 0; trial < 1900; ++trial)
    {
        const auto symbols = 1U + random() % 12;
        const auto a = make(random() % 33, symbols), b = make(random() % 35, symbols);
        std::vector<std::vector<std::size_t>> dp(a.size() + 1, std::vector<std::size_t>(b.size() + 1));
        std::iota(dp[0].begin(), dp[0].end(), std::size_t{0});
        for (std::size_t i = 0; i < a.size(); ++i)
        {
            dp[i + 1][0] = i + 1;
            for (std::size_t j = 0; j < b.size(); ++j)
            {
                dp[i + 1][j + 1] = std::min({dp[i][j] + (a[i] != b[j]), dp[i + 1][j] + 1, dp[i][j + 1] + 1});
            }
        }
        const auto expected = dp.back().back();
        assert(sirkit::edit_distance(a, b) == expected);
        for (std::size_t threshold = 0; threshold <= 35; ++threshold)
        {
            const auto actual = sirkit::edit_distance_at_most(a, b, threshold);
            assert(bool(actual) == (expected <= threshold));
            if (actual)
            {
                assert(*actual == expected);
            }
        }
    }
    assert(sirkit::edit_distance_at_most("", "a", std::numeric_limits<std::size_t>::max()) == 1);
    auto a = std::string(500000, 'q'), b = a;
    b.back() = 'x';
    assert(!sirkit::edit_distance_at_most(a, b, 0));
    assert(sirkit::edit_distance_at_most(a, b, 1) == 1);
}
