// SPDX-License-Identifier: CC0-1.0
#include "../String/Dynamic_Palindrome.cpp"
#include <cassert>

int main()
{
    sirkit::DynamicPalindrome text("abcba");
    assert(text.is_palindrome(0, 5));
    text.set(0, 'x');
    assert(text.get(0) == 'x');
    assert(!text.is_palindrome(0, 5));
    assert(text.is_palindrome(1, 4));
}
