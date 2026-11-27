// SPDX-License-Identifier: CC0-1.0
#include "../String/Palindromes_In_Deque.cpp"
#include <cassert>

int main()
{
    sirkit::PalindromesInDeque palindromes;
    palindromes.push_back('a');
    palindromes.push_front('b');
    palindromes.push_back('b');
    assert(palindromes.distinct_palindrome_count() == 3);
    assert(palindromes.longest_prefix_length() == 3);
    palindromes.pop_front();
    assert(palindromes.longest_suffix_length() == 1);
}
