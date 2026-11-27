// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_WILDCARD_PATTERN_MATCHING_DEFINED
#include "../../../Math/NTT.cpp"
#include "../../../String/Wildcard_Pattern_Matching.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
std::vector<bool> reference(std::string_view text, std::string_view pattern)
{
    std::vector<bool> result;
    while (text.size() >= pattern.size())
    {
        result.push_back(std::ranges::equal(text.substr(0, pattern.size()), pattern,
                                            [](char a, char b)
                                            {
                                                return a == b || a == '*' || b == '*';
                                            }));
        if (text.empty())
        {
            break;
        }
        text.remove_prefix(1);
    }
    return result;
}

void compare(std::string_view text, std::string_view pattern)
{
    assert(sirkit::wildcard_pattern_matching(text, pattern) == reference(text, pattern));
}
} // namespace

int main()
{
    std::vector<std::string> words{""};
    for (std::size_t i = 0; i < words.size(); ++i)
    {
        if (words[i].size() < 4)
        {
            const auto word = words[i];
            for (const char c : std::string_view("ab*"))
            {
                words.push_back(word + c);
            }
        }
    }
    for (const auto &text : words)
    {
        for (const auto &pattern : words)
        {
            compare(text, pattern);
        }
    }

    std::mt19937 random(104455);
    for (int trial = 0; trial < 1000; ++trial)
    {
        const auto fill = [&](std::size_t n)
        {
            std::string s(n, '*');
            for (auto &c : s)
            {
                if (random() % 3 != 0)
                {
                    c = static_cast<char>('a' + random() % 26);
                }
            }
            return s;
        };
        const auto text = fill(random() % 220);
        const auto pattern = fill(random() % 146);
        compare(text, pattern);
        if (text.size() > 33)
        {
            auto match = text.substr(1, text.size() - 2);
            for (char &c : match)
            {
                if (random() % 2 == 0)
                {
                    c = '*';
                }
            }
            compare(text, match);
        }
    }

#ifndef SKIP_RUNTIME_CHECKS
    for (const char c : std::string_view("AZ09?[]\x7f"))
    {
        bool threw = false;
        try
        {
            (void)sirkit::wildcard_pattern_matching("", std::string(1, c));
        }
        catch (const std::invalid_argument &)
        {
            threw = true;
        }
        assert(threw);
    }
#endif

    constexpr std::size_t kMaxLength = 1 << 19;
    bool too_long = false;
    try
    {
        (void)sirkit::wildcard_pattern_matching(std::string(kMaxLength + 1, '*'), "");
    }
    catch (const std::length_error &)
    {
        too_long = true;
    }
    assert(too_long);

    // a*b*(a-b)^2 を998244353法で足すと0になる非一致。二乗差のマスク方式では区別できる。
    constexpr std::size_t kTarget = 2 * 998244353;
    constexpr std::size_t kProduct = 26 * 9 * (26 - 9) * (26 - 9);
    constexpr auto kLarge = kTarget / kProduct, kSmall = kTarget % kProduct / 2;
    const auto collision_text = std::string(kLarge, 'z') + std::string(kSmall, 'a');
    const auto collision_pattern = std::string(kLarge, 'i') + std::string(kSmall, 'b');
    assert(sirkit::wildcard_pattern_matching(collision_text, collision_pattern) == std::vector<bool>{false});

    // 入力両方を上限にし、中央の1文字だけ異なる。
    std::string text(kMaxLength, 'z'), pattern(kMaxLength, 'z');
    pattern[kMaxLength / 2] = 'a';
    assert(sirkit::wildcard_pattern_matching(text, pattern) == std::vector<bool>{false});
    text[kMaxLength / 2] = '*';
    assert(sirkit::wildcard_pattern_matching(text, pattern) == std::vector<bool>{true});
}
