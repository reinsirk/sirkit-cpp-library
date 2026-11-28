// SPDX-License-Identifier: CC0-1.0
#include "String/Lexicographic_String_Shuffle.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
std::string exhaustive_shuffle(std::vector<std::string_view> suffixes)
{
    std::string best;
    for (auto &suffix : suffixes)
    {
        if (suffix.empty())
        {
            continue;
        }
        const char first = suffix.front();
        suffix.remove_prefix(1);
        const auto candidate = first + exhaustive_shuffle(suffixes);
        if (best.empty() || std::lexicographical_compare(candidate.begin(), candidate.end(), best.begin(), best.end(),
                                                         [](unsigned char a, unsigned char b)
                                                         {
                                                             return a < b;
                                                         }))
        {
            best = candidate;
        }
        suffix = std::string_view(suffix.data() - 1, suffix.size() + 1);
    }
    return best;
}
}

int main()
{
    using sirkit::lexicographic_string_shuffle;
    assert(lexicographic_string_shuffle({}).empty());
    assert(lexicographic_string_shuffle(std::array<std::string_view, 4>{"", "xyz", "", ""}) == "xyz");
    assert(lexicographic_string_shuffle(std::array<std::string_view, 2>{"b", "ba"}) == "bab");
    const std::string high{static_cast<char>(255), '\0'};
    const std::array<std::string_view, 3> binary{high, std::string_view(high).substr(1), ""};
    assert(lexicographic_string_shuffle(binary) == exhaustive_shuffle({binary.begin(), binary.end()}));
    const std::array<std::string_view, 7> binary_short{"", "a", "b", "aa", "ab", "ba", "bb"};
    for (const auto a : binary_short)
    {
        for (const auto b : binary_short)
        {
            for (const auto c : binary_short)
            {
                for (const auto d : binary_short)
                {
                    assert(lexicographic_string_shuffle(std::array{a, b, c, d}) == exhaustive_shuffle({a, b, c, d}));
                }
            }
        }
    }
    std::mt19937 random(205);
    for (int test = 0; test < 300; ++test)
    {
        std::vector<std::string> words(2 + random() % 3);
        std::vector<std::string_view> views;
        for (auto &word : words)
        {
            for (std::size_t i = 0, length = random() % 4; i < length; ++i)
            {
                word.push_back(static_cast<char>('a' + random() % 3));
            }
            views.push_back(word);
        }
        assert(lexicographic_string_shuffle(views) == exhaustive_shuffle(views));
    }

    std::vector<std::string> words(2500);
    std::vector<std::string_view> views;
    std::array<std::size_t, 26> histogram{};
    for (std::size_t i = 0; i < words.size(); ++i)
    {
        auto &word = words[i];
        for (std::size_t j = 0; j < 400; ++j)
        {
            const auto letter = (i + j) % 26;
            word.push_back(static_cast<char>('a' + letter));
            ++histogram[letter];
        }
        std::sort(word.begin(), word.end());
        views.push_back(word);
    }
    std::string expected;
    for (std::size_t i = 0; i < histogram.size(); ++i)
    {
        expected.append(histogram[i], static_cast<char>('a' + i));
    }
    assert(lexicographic_string_shuffle(views) == expected);

    constexpr auto limit = static_cast<std::size_t>(std::numeric_limits<int>::max()) - 2;
    assert(sirkit::detail::ExtendShuffleLength(limit - 2, 1) == limit);
    try
    {
        (void)sirkit::detail::ExtendShuffleLength(limit - 1, 1);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
}
