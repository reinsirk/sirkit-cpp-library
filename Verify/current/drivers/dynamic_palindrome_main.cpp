// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <string>

int main()
{
    std::size_t length = 0, query_count = 0;
    std::string input;
    verify_in >> length >> query_count >> input;
    sirkit::DynamicPalindrome text(input);
    for (std::size_t i = 0; i < query_count; ++i)
    {
        int operation = 0;
        std::size_t left = 0;
        verify_in >> operation >> left;
        if (operation == 1)
        {
            std::string byte;
            verify_in >> byte;
            text.set(left - 1, static_cast<std::uint8_t>(byte[0]));
        }
        else
        {
            std::size_t right = 0;
            verify_in >> right;
            verify_out << (text.is_palindrome(left - 1, right) ? "Yes\n" : "No\n");
        }
    }
}
