// SPDX-License-Identifier: CC0-1.0
#include "String/Regex_Language_Edit_Distance.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
std::vector<std::string> generate_words(std::string_view pattern, int limit)
{
    std::vector<std::string> words;
    std::string word;
    const auto enumerate = [&](const auto &self, std::size_t index) -> void
    {
        if (index == pattern.size())
        {
            words.push_back(word);
            return;
        }
        const char letter = pattern[index++];
        const char quantifier = index == pattern.size() ? '\0' : pattern[index];
        const auto previous_length = word.size();
        if (quantifier == '?' || quantifier == '*')
        {
            const int repeats = quantifier == '?' ? 1 : limit;
            for (int count = 0; count <= repeats; ++count)
            {
                self(self, index + 1);
                word.push_back(letter);
            }
        }
        else
        {
            word.push_back(letter);
            self(self, index);
        }
        word.resize(previous_length);
    };
    enumerate(enumerate, 0);
    return words;
}

std::size_t fixed_edit_distance(std::string_view a, std::string_view b)
{
    const auto stride = b.size() + 1;
    std::vector<std::size_t> distance((a.size() + 1) * stride);
    for (std::size_t i = 0; i <= a.size(); ++i)
    {
        distance[i * stride] = i;
    }
    for (std::size_t j = 0; j <= b.size(); ++j)
    {
        distance[j] = j;
    }
    for (std::size_t i = 1; i <= a.size(); ++i)
    {
        for (std::size_t j = 1; j <= b.size(); ++j)
        {
            distance[i * stride + j] = std::min({distance[(i - 1) * stride + j] + 1,
                                                 distance[i * stride + j - 1] + 1,
                                                 distance[(i - 1) * stride + j - 1] + (a[i - 1] != b[j - 1])});
        }
    }
    return distance.back();
}

std::size_t brute(std::string_view a, std::string_view b, int limit)
{
    const auto left = generate_words(a, limit), right = generate_words(b, limit);
    auto best = std::numeric_limits<std::size_t>::max();
    for (const auto &x : left)
    {
        for (const auto &y : right)
        {
            best = std::min(best, fixed_edit_distance(x, y));
            if (best == 0)
            {
                return best;
            }
        }
    }
    return best;
}
}

int main()
{
    using sirkit::regex_language_edit_distance;
    const std::vector<std::string> atoms{"a", "b", "a?", "b?", "a*", "b*"};
    std::vector<std::string> patterns{""};
    for (const auto &a : atoms)
    {
        patterns.push_back(a);
        for (const auto &b : atoms)
        {
            patterns.push_back(a + b);
        }
    }
    for (const auto &a : patterns)
    {
        for (const auto &b : patterns)
        {
            assert(regex_language_edit_distance(a, b) == brute(a, b, 2));
        }
    }
    std::mt19937 rng(2776);
    const auto pattern = [&]
    {
        std::string result;
        const auto count = rng() % 5;
        for (unsigned j = 0; j < count; ++j)
        {
            result += atoms[rng() % atoms.size()];
        }
        return result;
    };
    for (int test = 0; test < 700; ++test)
    {
        const auto a = pattern(), b = pattern();
        assert(regex_language_edit_distance(a, b) == brute(a, b, 4));
    }
    assert(regex_language_edit_distance("x*", std::string(2000, 'x')) == 0);
    for (const auto invalid : {"?a", "a**", "a?*", "A", "ab.", "a{"})
    {
        try
        {
            (void)regex_language_edit_distance("a", invalid);
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
}
