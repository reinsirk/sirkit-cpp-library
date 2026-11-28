// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Pattern_Erasure.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
void check(const std::string &text, const std::string &pattern)
{
    std::string expected(text);
    std::size_t removals = 0;
    while (true)
    {
        const auto first = expected.find(pattern);
        if (first == std::string::npos)
        {
            break;
        }
        expected.erase(first, pattern.size());
        ++removals;
    }
    assert(sirkit::erase_pattern(text, pattern) == expected);
    auto inplace = text;
    assert(sirkit::erase_pattern_inplace(inplace, pattern) == removals);
    assert(inplace == expected);
}

std::string binary_word(std::size_t length, std::uint16_t bits)
{
    std::string result(length, 'a');
    for (std::size_t i = 0; i < length; ++i)
    {
        if ((bits >> i) & 1U)
        {
            result[i] = 'b';
        }
    }
    return result;
}
}

int main()
{
    assert(sirkit::erase_pattern("", "A").empty());
    assert(sirkit::erase_pattern("ababa", "aba") == "ba");
    assert(sirkit::erase_pattern("ABCABC", "ABC").empty());
    assert(sirkit::erase_pattern("ababc", "abc") == "ab");

    for (std::size_t n = 0; n <= 8; ++n)
    {
        for (std::uint16_t text = 0; text < (1U << n); ++text)
        {
            for (std::size_t m = 1; m <= 5; ++m)
            {
                for (std::uint16_t pattern = 0; pattern < (1U << m); ++pattern)
                {
                    check(binary_word(n, text), binary_word(m, pattern));
                }
            }
        }
    }

    std::mt19937 gen(780392);
    constexpr std::array alphabet{'A', 'B', '\0', '\xff'};
    for (int round = 0; round < 1300; ++round)
    {
        std::string text(gen() % 80, 'A'), pattern(gen() % 10 + 1, 'A');
        for (auto &byte : text)
        {
            byte = alphabet[gen() % alphabet.size()];
        }
        for (auto &byte : pattern)
        {
            byte = alphabet[gen() % alphabet.size()];
        }
        check(text, pattern);
    }

    std::string alias = "xyxyxyz";
    const auto needle = std::string_view(alias).substr(2, 2);
    assert(sirkit::erase_pattern_inplace(alias, needle) == 3 && alias == "z");
    assert(sirkit::erase_pattern_inplace(alias, alias) == 1 && alias.empty());

    std::string unchanged = "abc";
    bool rejected = false;
    try
    {
        sirkit::erase_pattern_inplace(unchanged, "");
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected && unchanged == "abc");
}
