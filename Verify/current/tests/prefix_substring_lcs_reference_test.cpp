// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_PREFIX_SUBSTRING_LCS_DEFINED
#include "../../../String/Prefix_Substring_LCS.cpp"
#endif
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
std::vector<std::vector<std::size_t>> reference(std::string_view s, std::string_view t)
{
    std::vector<std::vector<std::size_t>> dp(s.size() + 1, std::vector<std::size_t>(t.size() + 1));
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        for (std::size_t j = 0; j < t.size(); ++j)
        {
            if (s[i] == t[j])
            {
                dp[i + 1][j + 1] = dp[i][j] + 1;
            }
            else
            {
                dp[i + 1][j + 1] = std::max(dp[i][j + 1], dp[i + 1][j]);
            }
        }
    }
    return dp;
}

void compare(std::string_view s, std::string_view t)
{
    const sirkit::PrefixSubstringLCS lcs(s, t);
    for (std::size_t b = 0; b <= t.size(); ++b)
    {
        const auto dp = reference(s, t.substr(b));
        for (std::size_t a = 0; a <= s.size(); ++a)
        {
            for (std::size_t c = b; c <= t.size(); ++c)
            {
                assert(lcs.length(a, b, c) == dp[a][c - b]);
            }
        }
    }
}

#ifndef SKIP_RUNTIME_CHECKS
void expect_invalid(const sirkit::PrefixSubstringLCS &lcs, std::size_t a, std::size_t b, std::size_t c)
{
    bool threw = false;
    try
    {
        (void)lcs.length(a, b, c);
    }
    catch (const std::out_of_range &)
    {
        threw = true;
    }
    assert(threw);
}
#endif

std::string word(std::mt19937 &random, std::size_t length, unsigned alphabet)
{
    std::string text(length, 'a');
    for (auto &c : text)
    {
        c = static_cast<char>(random() % alphabet);
    }
    return text;
}
} // namespace

int main()
{
    std::vector<std::string> words{""};
    for (std::size_t i = 0; i < words.size(); ++i)
    {
        if (words[i].size() < 5)
        {
            const auto text = words[i];
            words.push_back(text + 'a');
            words.push_back(text + 'b');
        }
    }
    for (const auto &s : words)
    {
        for (const auto &t : words)
        {
            compare(s, t);
        }
    }
    compare(std::string({'\0', static_cast<char>(0xff), 'c', '\0'}), std::string({'z', '\0', 'c', '\0'}));

#ifndef SKIP_RUNTIME_CHECKS
    const sirkit::PrefixSubstringLCS lcs("abc", "bc");
    expect_invalid(lcs, 4, 0, 0);
    expect_invalid(lcs, 0, 2, 1);
    expect_invalid(lcs, 0, 0, 3);
#endif

    std::mt19937 random(26879);
    for (const auto n : {31, 32, 33, 63, 64, 65, 127, 128, 129})
    {
        compare(word(random, 22, 7), word(random, n, 5));
    }
    for (int trial = 0; trial < 1000; ++trial)
    {
        compare(word(random, random() % 20, 1 + random() % 12), word(random, random() % 40, 1 + random() % 12));
    }

    constexpr std::size_t kMaxSize = 1'000;
    const auto s = word(random, kMaxSize, 11);
    const auto t = word(random, kMaxSize, 13);
    const sirkit::PrefixSubstringLCS maximum(s, t);
    constexpr std::array<std::size_t, 8> starts{0, 1, 63, 64, 300, 500, 990, 999};
    for (const auto b : starts)
    {
        const auto dp = reference(s, std::string_view(t).substr(b));
        for (int query = 0; query < 62'500; ++query)
        {
            const auto a = random() % (kMaxSize + 1);
            const auto c = b + random() % (kMaxSize - b + 1);
            assert(maximum.length(a, b, c) == dp[a][c - b]);
        }
    }
}
