// SPDX-License-Identifier: CC0-1.0
#include "String/Lexicographic_Concatenation.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace
{
std::string concatenate(std::span<const std::string_view> words, std::span<const std::size_t> order)
{
    std::string result;
    for (const auto i : order)
    {
        result.append(words[i]);
    }
    return result;
}

void check(std::span<const std::string_view> words)
{
    std::vector<std::string> expected;
    std::vector<std::size_t> order(words.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    do
    {
        expected.push_back(concatenate(words, order));
    } while (std::next_permutation(order.begin(), order.end()));
    std::sort(expected.begin(), expected.end());
    const auto actual_order = sirkit::lexicographic_concatenation_order(words);
    assert(std::is_permutation(actual_order.begin(), actual_order.end(), order.begin(), order.end()));
    assert(concatenate(words, actual_order) == expected.front());
    const auto second = sirkit::second_lexicographic_concatenation(words);
    if (expected.size() >= 2)
    {
        assert(second && *second == expected[1]);
    }
    else
    {
        assert(!second);
    }
}
}

int main()
{
    check({});
    check(std::array<std::string_view, 1>{"k"});
    check(std::array<std::string_view, 2>{"ab", "a"});
    check(std::array<std::string_view, 2>{"a", "ab"});
    check(std::array<std::string_view, 3>{"", "", ""});
    check(std::array<std::string_view, 3>{"abc", "ac", "ahc"});
    check(std::array<std::string_view, 3>{"a", "aab", "b"});
    check(std::array<std::string_view, 4>{"c", "b", "a", "d"});
    check(std::array<std::string_view, 4>{"", "b", "a", "a"});
    check(std::array<std::string_view, 3>{"xxy", "xxyxxy", "z"});
    const std::string high{static_cast<char>(255), '\0'};
    check(std::array<std::string_view, 3>{high, "x", std::string_view(high).substr(1)});
    const std::array<std::string_view, 9> choices{"", "a", "b", "aa", "ab", "ba", "bb", "aba", "aaa"};
    for (const auto a : choices)
    {
        for (const auto b : choices)
        {
            for (const auto c : choices)
            {
                for (const auto d : choices)
                {
                    check(std::array{a, b, c, d});
                }
            }
        }
    }

    std::mt19937 random(434);
    for (int test = 0; test < 500; ++test)
    {
        std::vector<std::string> words(random() % 7);
        std::vector<std::string_view> view;
        for (auto &word : words)
        {
            for (std::size_t i = 0, length = random() % 6; i < length; ++i)
            {
                word.push_back(static_cast<char>('a' + random() % 3));
            }
            view.push_back(word);
        }
        check(view);
    }

    constexpr auto limit = static_cast<std::size_t>(std::numeric_limits<int>::max()) - 2;
    assert(sirkit::detail::ExtendConcatenationLength(limit - 1, 1) == limit);
    try
    {
        (void)sirkit::detail::ExtendConcatenationLength(limit, 1);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
}
