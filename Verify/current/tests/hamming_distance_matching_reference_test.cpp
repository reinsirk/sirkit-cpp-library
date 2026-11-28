// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_HAMMING_DISTANCE_MATCHING_DEFINED
#include "../../../String/Hamming_Distance_Matching.cpp"
#endif
#include <cassert>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
void check(std::string_view text, std::string_view pattern, std::size_t allowance)
{
    std::vector<std::size_t> expected;
    std::vector<std::size_t> positions;
    for (std::size_t start = 0; start + pattern.size() <= text.size(); ++start)
    {
        auto mismatches = std::size_t{0};
        for (std::size_t i = 0; i < pattern.size(); ++i)
        {
            mismatches += text[start + i] != pattern[i];
        }
        expected.push_back(mismatches);
        if (mismatches <= allowance)
        {
            positions.push_back(start);
        }
    }
    assert(sirkit::hamming_distances(text, pattern) == expected);
    assert(sirkit::approximate_match_positions(text, pattern, allowance) == positions);
}
} // namespace

int main()
{
    check("", "", 0);
    check("", "a", 0);
    check("abc", "", 0);
    check("aaba", "ab", 1);
    check(std::string{char(255), '*', '\0', char(255), '*'}, std::string{char(255), '*', '\0'}, 1);
    std::mt19937 random(2373653);
    for (int trial = 0; trial < 450; ++trial)
    {
        std::string text(random() % 220, '\0');
        std::string pattern(random() % 130, '\0');
        const auto alphabet = random() % 70 + 1;
        for (auto &byte : text)
        {
            byte = static_cast<char>(random() % alphabet);
        }
        for (auto &byte : pattern)
        {
            byte = static_cast<char>(random() % (alphabet + 1));
        }
        check(text, pattern, random() % (pattern.size() + 1));
    }

    std::string heavy_text(4096, 'a'), heavy_pattern(930, 'a');
    heavy_text[0] = heavy_text[200] = heavy_text.back() = 'b';
    heavy_pattern[130] = 'b';
    heavy_text[304] = 'c';
    heavy_pattern[330] = 'd';
    check(heavy_text, heavy_pattern, 0);
    check(heavy_text, heavy_pattern, 2);

    constexpr auto kCapacity = sirkit::NTT<>::max_length();
    for (const auto text_size : {kCapacity, kCapacity + 1})
    {
        bool thrown = false;
        try
        {
            (void)sirkit::detail::hamming_transform_size(text_size, 2);
        }
        catch (const std::length_error &)
        {
            thrown = true;
        }
        assert(thrown);
    }
    assert(sirkit::detail::hamming_transform_size(kCapacity, 1) == kCapacity);
}
