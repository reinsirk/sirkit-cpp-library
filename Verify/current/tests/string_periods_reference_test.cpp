// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_STRING_PERIODS_DEFINED
#include "../../../String/String_Periods.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
void check(std::string_view text)
{
    const sirkit::StringPeriods index(text);
    assert(index.size() == text.size());
    assert(index.least_period(0) == 0 && index.primitive_root_length(0) == 0);
    assert(index.periods(0).empty() && index.proper_borders(0).empty());
    for (std::size_t length = 1; length <= text.size(); ++length)
    {
        const auto prefix = text.substr(0, length);
        std::vector<std::size_t> periods;
        auto root = length;
        for (std::size_t candidate = 1; candidate <= length; ++candidate)
        {
            if (prefix.substr(candidate) == prefix.substr(0, length - candidate))
            {
                periods.push_back(candidate);
                if (length % candidate == 0)
                {
                    root = std::min(root, candidate);
                }
            }
        }
        std::vector<std::size_t> borders;
        for (std::size_t candidate = length - 1; candidate > 0; --candidate)
        {
            if (prefix.starts_with(prefix.substr(length - candidate)))
            {
                borders.push_back(candidate);
            }
        }
        assert(index.periods(length) == periods);
        assert(index.proper_borders(length) == borders);
        assert(index.least_period(length) == periods.front());
        assert(index.primitive_root_length(length) == root);
    }
}

template<class Exception, class Function>
void check_error(Function run)
{
    bool thrown = false;
    try
    {
        run();
    }
    catch (const Exception &)
    {
        thrown = true;
    }
    assert(thrown);
}
} // namespace

int main()
{
    check("");
    check("a");
    check("abababa");
    check("ababab");
    check("aabaabaa");
    check("aaaaaaaaa");
    check("abcdefghij");
    const std::string bytes{char(255), '\0', char(255), '\0', char(255)};
    check(bytes);

    std::mt19937 random(562823);
    for (int trial = 0; trial < 1200; ++trial)
    {
        const auto alphabet = random() % 8 + 1;
        std::string text(random() % 57, '\0');
        for (auto &byte : text)
        {
            byte = static_cast<char>(random() % alphabet);
        }
        check(text);
    }

    constexpr std::size_t kLength = 500'000;
    const sirkit::StringPeriods largest(std::string(kLength, 'x'));
    std::vector<std::size_t> expected(kLength);
    std::iota(expected.begin(), expected.end(), std::size_t{1});
    assert(largest.size() == kLength);
    assert(largest.periods(kLength) == expected);
    for (std::size_t length = 1; length <= kLength; ++length)
    {
        assert(largest.primitive_root_length(length) == 1);
    }
#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::out_of_range>([&] { (void)largest.periods(kLength + 1); });
#endif

    // 同梱したPrefix_Function.cppの公開APIも単独で利用できる。
    assert((sirkit::find_occurrences("ab", "") == std::vector<std::size_t>{0, 1, 2}));
    assert((sirkit::find_occurrences("abababaca", "ababaca") == std::vector<std::size_t>{2}));
    assert((sirkit::find_occurrences("xaaaxaaaa", "aaa") == std::vector<std::size_t>{1, 5, 6}));
    check_error<std::length_error>([] { (void)sirkit::detail::occurrence_boundary_count(1, 1); });
}
