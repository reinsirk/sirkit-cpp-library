// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_SUBSTRING_INDEX_DEFINED
#include "../../../String/Substring_Index.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace
{
void check_occurrences(const sirkit::SubstringIndex &index, std::string_view text, std::string_view pattern)
{
    std::vector<int> expected;
    for (std::size_t start = 0; start < text.size(); ++start)
    {
        if (text.substr(start).starts_with(pattern))
        {
            expected.push_back(static_cast<int>(start));
        }
    }
    std::ranges::sort(expected, [text](int a, int b) {
        return text.substr(static_cast<std::size_t>(a)) < text.substr(static_cast<std::size_t>(b));
    });
    assert(std::ranges::equal(index.occurrences(pattern), expected));
}

void compare(std::string_view text)
{
    const sirkit::SubstringIndex index(text);
    std::set<std::string_view> expected;
    for (std::size_t i = 0; i < text.size(); ++i)
    {
        for (std::size_t length = 1; length <= text.size() - i; ++length)
        {
            expected.insert(text.substr(i, length));
        }
    }
    assert(index.distinct_substring_count() == expected.size());
    assert(!index.kth_distinct_substring(0));
    assert(!index.kth_distinct_substring(expected.size() + 1));
    assert(!index.kth_distinct_substring(std::numeric_limits<std::uint64_t>::max()));
    std::uint64_t rank = 0;
    for (std::string_view pattern : expected)
    {
        ++rank;
        assert(index.kth_distinct_substring(rank).value() == pattern);
        check_occurrences(index, text, pattern);
    }
    check_occurrences(index, text, "");
    check_occurrences(index, text, "missing");
    check_occurrences(index, text, "zzzzzz");
}
} // namespace

int main()
{
    compare("");
    compare("a");
    compare("aa");
    compare("banana");
    compare("abababab");
    const std::string binary{char(255), '\0', char(127), char(255)};
    compare(binary);
    std::mt19937 random(15307);
    for (int trial = 0; trial < 750; ++trial)
    {
        std::string text(random() % 32, '\0');
        for (char &byte : text)
        {
            byte = static_cast<char>(random() % (2U << (trial % 8)));
        }
        compare(text);
    }

    const std::string repeated(500'000, 'a');
    const sirkit::SubstringIndex large(repeated);
    assert(large.distinct_substring_count() == repeated.size());
    assert(large.kth_distinct_substring(500'000).value() == repeated);
    assert(large.occurrences("aa").size() == repeated.size() - 1);

    std::string source = "ababa";
    sirkit::SubstringIndex owned(source);
    source.assign("xxxxx");
    const auto matches = owned.occurrences("aba");
    const auto part = owned.kth_distinct_substring(3).value();
    const sirkit::SubstringIndex copy = owned;
    owned = large;
    assert(matches.size() == 2 && matches.front() == 2 && matches.back() == 0);
    assert(part == "aba");
    assert(copy.distinct_substring_count() == 9);
    assert(owned.distinct_substring_count() == repeated.size());
}
