// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Longest_Common_Subsequence.cpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <random>
#include <string>
#include <string_view>
#include <vector>

int main()
{
    std::mt19937 random(397);
    const auto make = [&](unsigned n, unsigned alphabet)
    {
        std::string result(n, 'a');
        for (auto &letter : result)
        {
            letter += random() % alphabet;
        }
        return result;
    };
    const auto subsequence = [](std::string_view text, std::string_view what)
    {
        for (auto letter : text)
        {
            if (!what.empty() && letter == what.front())
            {
                what.remove_prefix(1);
            }
        }
        return what.empty();
    };
    for (int trial = 0; trial < 3200; ++trial)
    {
        const auto symbols = 1U + random() % 9;
        const auto a = make(random() % 63, symbols), b = make(random() % 68, symbols);
        std::vector<std::vector<std::size_t>> dp(a.size() + 1, std::vector<std::size_t>(b.size() + 1));
        for (std::size_t i = 0; i < a.size(); ++i)
        {
            for (std::size_t j = 0; j < b.size(); ++j)
            {
                dp[i + 1][j + 1] = a[i] == b[j] ? dp[i][j] + 1 : std::max(dp[i][j + 1], dp[i + 1][j]);
            }
        }
        const auto result = sirkit::longest_common_subsequence(a, b);
        assert(result.size() == dp.back().back());
        assert(subsequence(a, result) && subsequence(b, result));
    }
    assert(sirkit::longest_common_subsequence(std::string(3000, 'x'), "a").empty());
}
