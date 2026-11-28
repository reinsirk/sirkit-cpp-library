// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_SHORTEST_COMMON_SUPERSTRING_DEFINED
#include "../../../String/Shortest_Common_Superstring.cpp"
#endif
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
bool covers(std::string_view text, std::span<const std::string_view> words)
{
    return std::ranges::all_of(words, [&](std::string_view word) { return text.contains(word); });
}

void check(std::span<const std::string_view> words, std::size_t expected_length)
{
    const auto answer = sirkit::shortest_common_superstring(words);
    assert(answer.size() == expected_length);
    assert(covers(answer, words));
}

std::size_t brute_length(std::span<const std::string_view> words)
{
    std::size_t lower = 0, upper = 0;
    for (const auto word : words)
    {
        lower = std::max(lower, word.size());
        upper += word.size();
    }
    for (auto length = lower; length < upper; ++length)
    {
        std::string candidate(length, 'a');
        for (std::size_t mask = 0; mask < (std::size_t{1} << length); ++mask)
        {
            for (std::size_t i = 0; i < length; ++i)
            {
                candidate[i] = (mask >> i) & 1U ? 'b' : 'a';
            }
            if (covers(candidate, words))
            {
                return length;
            }
        }
    }
    return upper;
}

template<class Function>
void check_length_error(Function run)
{
    bool thrown = false;
    try
    {
        run();
    }
    catch (const std::length_error &)
    {
        thrown = true;
    }
    assert(thrown);
}
} // namespace

int main()
{
    using namespace std::string_view_literals;
    check({}, 0);
    check(std::array{""sv, ""sv}, 0);
    check(std::array{""sv, "abc"sv, "abc"sv, "bc"sv, "b"sv}, 3);
    check(std::array{"ab"sv, "bc"sv}, 3);
    check(std::array{"snuke"sv, "kensho"sv, "uk"sv}, 9);
    check(std::array{"abab"sv, "abba"sv, "abb"sv}, 6);
    check(std::array{"\0b"sv, "b\0"sv, ""sv}, 3);
    check(std::array{"annaaa"sv, "anana"sv, "nnaa"sv}, 10);
    std::mt19937 random(434950);
    for (int trial = 0; trial < 400; ++trial)
    {
        std::vector<std::string> input(random() % 6);
        std::vector<std::string_view> words;
        for (auto &word : input)
        {
            word.resize(random() % 5);
            for (auto &byte : word)
            {
                byte = random() % 2 == 0 ? 'a' : 'b';
            }
            words.push_back(word);
        }
        check(words, brute_length(words));
    }

    constexpr std::size_t kSize = 13, kRun = 500;
    std::array<std::string, kSize> input;
    std::array<std::string_view, kSize> words;
    for (std::size_t i = 0; i < kSize; ++i)
    {
        input[i] = std::string(kRun, static_cast<char>('a' + i)) + std::string(kRun, static_cast<char>('b' + i));
        words[i] = input[i];
    }
    std::ranges::reverse(words);
    check(words, (kSize + 1) * kRun);
    assert((sirkit::find_occurrences("ab", "") == std::vector<std::size_t>{0, 1, 2}));
    assert((sirkit::find_occurrences("abababaca", "ababaca") == std::vector<std::size_t>{2}));
    check_length_error([] { (void)sirkit::shortest_common_superstring(std::array<std::string_view, 21>{}); });
    check_length_error([] {
        (void)sirkit::detail::superstring_add_length(std::numeric_limits<std::uint32_t>::max(), 1);
    });
}
