// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_MULTI_STRING_LONGEST_COMMON_SUBSTRING_DEFINED
#include "../../../String/Multi_String_Longest_Common_Substring.cpp"
#endif
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
std::size_t naive_lcs_length(std::span<const std::string_view> texts)
{
    if (texts.empty())
    {
        return 0;
    }
    const auto shortest = *std::ranges::min_element(texts, {}, &std::string_view::size);
    for (std::size_t length = shortest.size(); length > 0; --length)
    {
        for (std::size_t start = 0; start + length <= shortest.size(); ++start)
        {
            const auto part = shortest.substr(start, length);
            if (std::ranges::all_of(texts, [part](auto text) { return text.find(part) != std::string_view::npos; }))
            {
                return length;
            }
        }
    }
    return 0;
}

void compare(std::span<const std::string_view> texts)
{
    const auto result = sirkit::longest_common_substring(texts);
    assert(result.starts.size() == texts.size());
    assert(result.length == naive_lcs_length(texts));
    if (result.length == 0)
    {
        assert(std::ranges::all_of(result.starts, [](auto start) { return start == 0; }));
        return;
    }
    const auto part = texts.front().substr(result.starts.front(), result.length);
    assert(part.size() == result.length);
    for (std::size_t i = 0; i < texts.size(); ++i)
    {
        assert(texts[i].substr(result.starts[i], result.length) == part);
    }
}
} // namespace

int main()
{
    compare({});
    compare(std::array<std::string_view, 1>{"test"});
    compare(std::array<std::string_view, 3>{"banana", "", "ana"});
    compare(std::array<std::string_view, 3>{"ab", "cd", "ef"});
    compare(std::array<std::string_view, 3>{"ababa", "xabax", "aba"});
    compare(std::array<std::string_view, 3>{"aaaabb", "aaa", "zaaaa"});
    const std::string binary{char(255), '\0', char(127), char(255)};
    compare(std::array<std::string_view, 3>{binary, std::string_view(binary).substr(1), binary});

    std::mt19937 random(523047);
    for (int trial = 0; trial < 1500; ++trial)
    {
        std::vector<std::string> owned(2 + random() % 5);
        std::vector<std::string_view> texts;
        const unsigned alphabet = 1U << (trial % 8);
        for (auto &text : owned)
        {
            text.resize(1 + random() % 22);
            for (auto &byte : text)
            {
                byte = static_cast<char>(random() % alphabet);
            }
            texts.emplace_back(text);
        }
        compare(texts);
    }

    const std::vector<std::string_view> many(100'000, "a");
    const auto one = sirkit::longest_common_substring(many);
    assert(one.length == 1);
    assert(std::ranges::all_of(one.starts, [](auto start) { return start == 0; }));

    const std::string repeated(300'000, 'a');
    const auto shorter = std::string_view(repeated).substr(1);
    const auto large = sirkit::longest_common_substring(std::array<std::string_view, 3>{repeated, shorter, repeated});
    assert(large.length == shorter.size());
    assert(large.starts[0] <= 1 && large.starts[1] == 0 && large.starts[2] <= 1);

    constexpr auto kLimit = static_cast<std::size_t>(std::numeric_limits<int>::max()) - 2;
    assert(sirkit::detail::shared_substring_extend_length(kLimit - 2, 1) == kLimit);
    bool thrown = false;
    try
    {
        (void)sirkit::detail::shared_substring_extend_length(kLimit - 1, 1);
    }
    catch (const std::length_error &)
    {
        thrown = true;
    }
    assert(thrown);
}
