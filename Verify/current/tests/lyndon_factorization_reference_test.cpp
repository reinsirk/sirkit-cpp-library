// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_LYNDON_FACTORIZATION_TEST_SOURCE
#define SIRKIT_LYNDON_FACTORIZATION_TEST_SOURCE "../../../String/Lyndon_Factorization.cpp"
#endif
#include SIRKIT_LYNDON_FACTORIZATION_TEST_SOURCE
#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace
{
std::vector<std::size_t> EnumerateCuts(std::string_view text)
{
    for (unsigned mask = 0; mask < (1U << text.size()); ++mask)
    {
        std::vector<std::size_t> cuts{0};
        std::string_view previous;
        bool valid = true;
        for (std::size_t i = 1; i <= text.size() && valid; ++i)
        {
            if (i < text.size() && ((mask >> (i - 1)) & 1U) == 0)
            {
                continue;
            }
            const auto factor = text.substr(cuts.back(), i - cuts.back());
            if (!previous.empty() && previous < factor)
            {
                valid = false;
            }
            for (std::size_t j = 1; j < factor.size(); ++j)
            {
                if (factor >= factor.substr(j))
                {
                    valid = false;
                }
            }
            cuts.push_back(i);
            previous = factor;
        }
        if (valid)
        {
            return cuts;
        }
    }
    assert(false);
    return {};
}
} // namespace

int main()
{
    for (int n = 0, combinations = 1; n <= 8; ++n, combinations *= 3)
    {
        for (int pattern = 0; pattern < combinations; ++pattern)
        {
            std::string text(n, 'a');
            auto digits = pattern;
            for (char &c : text)
            {
                c = static_cast<char>('a' + digits % 3);
                digits /= 3;
            }
            assert(sirkit::lyndon_factorization(text) == EnumerateCuts(text));
        }
    }
    const std::string bytes{static_cast<char>(0xff), '\0', static_cast<char>(0x80)};
    assert((sirkit::lyndon_factorization(bytes) == std::vector<std::size_t>{0, 1, 3}));
    const std::string equal(500'000, 'a');
    const auto boundaries = sirkit::lyndon_factorization(equal);
    assert(boundaries.size() == equal.size() + 1);
    for (std::size_t i = 0; i < boundaries.size(); ++i)
    {
        assert(boundaries[i] == i);
    }
}
