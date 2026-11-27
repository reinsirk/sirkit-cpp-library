// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_STRING_RUN_DEFINED
#include "../../../String/Run_Enumerate.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace
{
std::vector<sirkit::StringRun> reference_runs(std::string_view s)
{
    std::vector<sirkit::StringRun> result;
    for (std::size_t l = 0; l < s.size(); ++l)
    {
        for (auto r = l + 2; r <= s.size(); ++r)
        {
            for (std::size_t p = 1; p <= (r - l) / 2; ++p)
            {
                if (s.substr(l, r - l - p) == s.substr(l + p, r - l - p))
                {
                    const bool extends_left = l != 0 && s[l - 1] == s[l + p - 1];
                    const bool extends_right = r < s.size() && s[r] == s[r - p];
                    if (!extends_left && !extends_right)
                    {
                        result.push_back({p, l, r});
                    }
                    break;
                }
            }
        }
    }
    std::ranges::sort(result);
    return result;
}

void compare(std::string_view text)
{
    assert(sirkit::run_enumerate(text) == reference_runs(text));
}
} // namespace

int main()
{
    compare("");
    compare("x");
    compare("aaaaaaaaaaaaaaaaaaaaaa");
    compare("abababa");
    compare("abcdabcdbaabcdabcdba");
    for (std::size_t n = 0; n <= 11; ++n)
    {
        for (std::size_t bits = 0; bits < (std::size_t{1} << n); ++bits)
        {
            std::string text(n, 'a');
            for (std::size_t i = 0; i < n; ++i)
            {
                if ((bits >> i) & 1)
                {
                    text[i] = 'b';
                }
            }
            compare(text);
        }
    }
    for (std::size_t code = 0; code < 2187; ++code)
    {
        std::string text(7, '\0');
        auto current = code;
        for (auto &c : text)
        {
            const auto digit = current % 3;
            c = static_cast<char>(digit * 127);
            current /= 3;
        }
        compare(text);
    }

    std::mt19937 random(42664);
    for (int trial = 0; trial < 2500; ++trial)
    {
        std::string text(random() % 47, '\0');
        const unsigned alphabet = 1U << (trial % 9);
        for (auto &c : text)
        {
            c = static_cast<char>(random() % alphabet);
        }
        compare(text);
    }

    const std::string equal(200000, 'a');
    assert((sirkit::run_enumerate(equal) == std::vector<sirkit::StringRun>{{1, 0, equal.size()}}));
    auto alternating = equal;
    for (std::size_t i = 1; i < alternating.size(); i += 2)
    {
        alternating[i] = 'b';
    }
    assert((sirkit::run_enumerate(alternating) == std::vector<sirkit::StringRun>{{2, 0, alternating.size()}}));
}
