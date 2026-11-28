// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_SUBSTRING_COMPARISON_DEFINED
#include "../../../String/Substring_Comparison.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <compare>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
std::size_t prefix(std::string_view a, std::string_view b)
{
    const auto [left, right] = std::mismatch(a.begin(), a.end(), b.begin(), b.end());
    (void)right;
    return static_cast<std::size_t>(left - a.begin());
}

void check(std::string_view text, std::mt19937 &random)
{
    const sirkit::SubstringComparison index(text);
    auto other = index;
    other = sirkit::SubstringComparison("");
    assert(other.size() == 0);
    other = index;
    assert(index.size() == text.size() && other.size() == text.size());
    for (std::size_t i = 0; i <= text.size(); ++i)
    {
        for (std::size_t j = 0; j <= text.size(); ++j)
        {
            const auto left = text.substr(i);
            const auto right = text.substr(j);
            const auto expected = prefix(left, right);
            assert(other.common_prefix(i, j) == expected);
            const auto len = random() % (std::min(left.size(), right.size()) + 1);
            assert(index.equal(i, j, len) == (left.substr(0, len) == right.substr(0, len)));
            const auto a = left.substr(0, random() % (left.size() + 1));
            const auto b = right.substr(0, random() % (right.size() + 1));
            const auto order = std::lexicographical_compare_three_way(
                a.begin(), a.end(), b.begin(), b.end(),
                [](unsigned char x, unsigned char y) { return x <=> y; });
            assert(index.compare(i, a.size(), j, b.size()) == order);
        }
    }
}

template<class Function>
void check_error(Function run)
{
    bool thrown = false;
    try
    {
        run();
    }
    catch (const std::out_of_range &)
    {
        thrown = true;
    }
    assert(thrown);
}
} // namespace

int main()
{
    std::mt19937 random(394878);
    check("", random);
    check("x", random);
    check("xyxyxyxyz", random);
    check("aaaaaaaaa", random);
    check("bananabanana", random);
    check(std::string{char(255), '\0', char(128), '\0', char(255), '\0'}, random);
    for (int i = 0; i < 180; ++i)
    {
        std::string text(random() % 58, '\0');
        const auto alphabet = random() % 15 + 2;
        for (auto &byte : text)
        {
            byte = static_cast<char>(random() % alphabet);
        }
        check(text, random);
    }

    constexpr std::size_t kLength = 200'000;
    std::string periodic(kLength, 'a');
    for (std::size_t i = 1; i < kLength; i += 2)
    {
        periodic[i] = 'b';
    }
    const sirkit::SubstringComparison large(periodic);
    for (std::size_t i = 0; i < kLength; ++i)
    {
        assert(large.common_prefix(0, i) == (i % 2 == 0 ? kLength - i : 0));
    }

#ifndef SKIP_RUNTIME_CHECKS
    check_error([&] { (void)large.common_prefix(kLength + 1, 0); });
    check_error([&] { (void)large.equal(0, kLength, 1); });
    check_error([&] { (void)large.compare(0, 0, 1, std::numeric_limits<std::size_t>::max()); });
#endif
}
