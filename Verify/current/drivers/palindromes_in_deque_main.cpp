// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>

int main()
{
    std::size_t q{};
    verify_in >> q;
    sirkit::PalindromesInDeque palindromes(q);
    while (q-- != 0)
    {
        int type{};
        verify_in >> type;
        if (type < 2)
        {
            char letter{};
            verify_in >> letter;
            if (type == 0)
            {
                palindromes.push_front(letter);
            }
            else
            {
                palindromes.push_back(letter);
            }
        }
        else if (type == 2)
        {
            palindromes.pop_front();
        }
        else
        {
            palindromes.pop_back();
        }
        verify_out << palindromes.distinct_palindrome_count() << ' '
                   << palindromes.longest_prefix_length() << ' ' << palindromes.longest_suffix_length() << '\n';
    }
}
