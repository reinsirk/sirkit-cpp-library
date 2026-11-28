// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Shortest_Absent_Subsequence.cpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
bool exists(std::string_view text, std::string_view subsequence)
{
    auto where = text.begin();
    for (const auto byte : subsequence)
    {
        where = std::find(where, text.end(), byte);
        if (where == text.end())
        {
            return false;
        }
        ++where;
    }
    return true;
}

std::string brute_force(std::string_view text, std::string_view alphabet)
{
    std::vector<unsigned char> codes(alphabet.begin(), alphabet.end());
    std::sort(codes.begin(), codes.end());
    codes.erase(std::unique(codes.begin(), codes.end()), codes.end());
    std::vector<std::string> layer{std::string{}};
    for (;;)
    {
        std::vector<std::string> next;
        for (const auto &prefix : layer)
        {
            for (const auto byte : codes)
            {
                auto candidate = prefix + static_cast<char>(byte);
                if (!exists(text, candidate))
                {
                    return candidate;
                }
                next.push_back(candidate);
            }
        }
        layer.swap(next);
    }
}
} // namespace

int main()
{
    const std::string binary_alphabet{static_cast<char>(0xff), '\0', 'a', '\0'};
    const std::array input_codes{'a', '\0', static_cast<char>(0xff), 'x'};
    const std::array<std::string_view, 5> alphabets{"a", "aax", "xa", "xya", binary_alphabet};
    std::mt19937 rng(435036);
    for (std::size_t trial = 0; trial < 600; ++trial)
    {
        const auto alphabet = alphabets[trial % alphabets.size()];
        std::string text(trial % 17, '\0');
        for (auto &byte : text)
        {
            byte = input_codes[rng() % input_codes.size()];
        }
        const auto expected = brute_force(text, alphabet);
        assert(sirkit::shortest_absent_subsequence(text, alphabet) == expected);
        assert(sirkit::shortest_absent_subsequence_length(text, alphabet) == expected.size());
    }
    assert(sirkit::shortest_absent_subsequence("abcdefghijklmnopqrstuvwxyz") == "aa");
    assert(sirkit::shortest_absent_subsequence_length("abcdefghijklmnopqrstuvwxyz") == 2);
    assert(sirkit::shortest_absent_subsequence("") == "a");
    bool empty_alphabet = false;
    try
    {
        static_cast<void>(sirkit::shortest_absent_subsequence("abc", ""));
    }
    catch (const std::invalid_argument &)
    {
        empty_alphabet = true;
    }
    assert(empty_alphabet);
    bool too_long = false;
    try
    {
        static_cast<void>(sirkit::detail::absent_subsequence_slots(std::vector<std::size_t>().max_size()));
    }
    catch (const std::length_error &)
    {
        too_long = true;
    }
    assert(too_long);
}
