// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Prefix_Function.cpp"
#include <cassert>
#include <random>
#include <string>
#include <string_view>
#include <vector>

int main()
{
    try
    {
        sirkit::detail::occurrence_boundary_count(10, 10);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    std::mt19937 random(203);
    const auto make = [&](std::size_t length, unsigned alphabet)
    {
        std::string result(length, 'a');
        for (auto &c : result)
        {
            c += random() % alphabet;
        }
        return result;
    };
    for (int trial = 0; trial < 3000; ++trial)
    {
        const auto alphabet = 1U + random() % 5;
        const auto text = make(random() % 58, alphabet);
        const auto pattern = make(random() % 28, alphabet);
        const auto borders = sirkit::prefix_function(text);
        std::vector<std::size_t> found;
        for (std::size_t i = 0; i < text.size(); ++i)
        {
            const auto prefix = std::string_view(text).substr(0, i + 1);
            std::size_t expected = 0;
            for (std::size_t length = 1; length < prefix.size(); ++length)
            {
                if (prefix.ends_with(prefix.substr(0, length)))
                {
                    expected = length;
                }
            }
            assert(borders[i] == expected);
        }
        for (std::size_t i = 0; i <= text.size(); ++i)
        {
            if (std::string_view(text).substr(i).starts_with(pattern))
            {
                found.push_back(i);
            }
        }
        assert(sirkit::find_occurrences(text, pattern) == found);
    }
    const std::string binary("a\0a\0a", 5);
    assert((sirkit::find_occurrences(binary, std::string_view(binary).substr(0, 3)) == std::vector<std::size_t>{0, 2}));
}
