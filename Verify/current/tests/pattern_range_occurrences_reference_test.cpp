#include "../../../String/Pattern_Range_Occurrences.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
using sirkit::PatternRangeOccurrences;

void verify(const PatternRangeOccurrences &index, std::string_view text, std::string_view pattern,
            std::size_t left, std::size_t right)
{
    std::vector<std::size_t> expected;
    for (std::size_t start = left; start <= right; ++start)
    {
        if (pattern.size() <= right - start && text.substr(start, pattern.size()) == pattern)
        {
            expected.push_back(start);
        }
    }
    assert(index.count(left, right) == expected.size());
    assert(std::ranges::equal(index.positions(left, right), expected));
}

std::string binary_word(std::mt19937 &random, std::size_t length)
{
    std::string word(length, 'a');
    for (auto &character : word)
    {
        character = static_cast<char>('a' + random() % 2);
    }
    return word;
}

template <typename Exception, typename Action>
void expect_exception(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}
}

int main()
{
    const PatternRangeOccurrences default_constructed;
    assert(default_constructed.size() == 0 && default_constructed.pattern_size() == 0);
    verify(default_constructed, "", "", 0, 0);
    verify(PatternRangeOccurrences("", "a"), "", "a", 0, 0);
    verify(PatternRangeOccurrences("", ""), "", "", 0, 0);
    verify(PatternRangeOccurrences("aaa", ""), "aaa", "", 0, 3);
    verify(PatternRangeOccurrences("aaa", ""), "aaa", "", 3, 3);
    verify(PatternRangeOccurrences("a", "aaaaa"), "a", "aaaaa", 0, 1);
    verify(PatternRangeOccurrences("aaaaa", "aaa"), "aaaaa", "aaa", 0, 5);
    verify(PatternRangeOccurrences("aaaaa", "aaa"), "aaaaa", "aaa", 1, 3);
    verify(PatternRangeOccurrences("aaaaa", "aaa"), "aaaaa", "aaa", 1, 5);
    constexpr std::string_view raw_text{"\0a\0a\0", 5};
    constexpr std::string_view raw_pattern{"\0a", 2};
    verify(PatternRangeOccurrences(raw_text, raw_pattern), raw_text, raw_pattern, 0, 5);

    PatternRangeOccurrences survivor;
    std::span<const std::size_t> borrowed;
    {
        const PatternRangeOccurrences temporary(std::string{"ababa"}, std::string{"aba"});
        borrowed = temporary.positions(0, 5);
        survivor = temporary;
    }
    auto moved = std::move(survivor);
    assert(moved.size() == 5 && moved.pattern_size() == 3);
    assert(survivor.size() == 0 && survivor.pattern_size() == 0 && survivor.count(0, 0) == 1);
    assert(std::ranges::equal(borrowed, std::array<std::size_t, 2>{0, 2}));
    assert(moved.count(0, 5) == 2);

    std::mt19937 random(477031);
    for (int test = 0; test < 1600; ++test)
    {
        const auto text = binary_word(random, random() % 56);
        const auto pattern = binary_word(random, random() % 19);
        const PatternRangeOccurrences index(text, pattern);
        assert(index.size() == text.size() && index.pattern_size() == pattern.size());
        for (int query = 0; query < 70; ++query)
        {
            const auto a = static_cast<std::size_t>(random() % (text.size() + 1));
            const auto b = static_cast<std::size_t>(random() % (text.size() + 1));
            verify(index, text, pattern, std::min(a, b), std::max(a, b));
        }
    }

    expect_exception<std::length_error>([] { (void)sirkit::detail::PatternRangeOffsetCount(0, 1); });
    expect_exception<std::length_error>([] { (void)sirkit::detail::PatternRangeOffsetCount(3, 4); });
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::out_of_range>([&] { (void)moved.count(5, 4); });
    expect_exception<std::out_of_range>([&] { (void)moved.positions(0, 6); });
#endif
}
