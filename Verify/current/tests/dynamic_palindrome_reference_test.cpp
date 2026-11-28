// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_DYNAMIC_PALINDROME_DEFINED
#include "../../../String/Dynamic_Palindrome.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace
{
bool naive_palindrome(std::string_view text, std::size_t left, std::size_t right)
{
    const auto part = text.substr(left, right - left);
    return std::equal(part.begin(), part.end(), part.rbegin(), part.rend());
}

template<class Exception, class Function>
void expect_throw(Function test)
{
    bool thrown = false;
    try
    {
        test();
    }
    catch (const Exception &)
    {
        thrown = true;
    }
    assert(thrown);
}
}

int main()
{
    const sirkit::DynamicPalindrome empty("");
    assert(empty.size() == 0 && empty.is_palindrome(0, 0));
    std::mt19937_64 random(19723);
    for (std::size_t n = 0; n <= 80; ++n)
    {
        std::string reference(n, '\0');
        sirkit::DynamicPalindrome text(reference, random());
        for (std::size_t step = 0; step < 140; ++step)
        {
            if (n != 0)
            {
                const std::size_t index = random() % n;
                const auto byte = static_cast<std::uint8_t>(random());
                reference[index] = static_cast<char>(byte);
                text.set(index, byte);
                text.set(index, byte);
                assert(text.get(index) == byte);
            }
            auto left = random() % (n + 1);
            auto right = random() % (n + 1);
            if (right < left)
            {
                std::swap(left, right);
            }
            assert(text.is_palindrome(left, right) == naive_palindrome(reference, left, right));
        }
        for (std::size_t left = 0; left <= n; ++left)
        {
            for (std::size_t right = left; right <= n; ++right)
            {
                assert(text.is_palindrome(left, right) == naive_palindrome(reference, left, right));
            }
        }

        auto copy = text;
        sirkit::DynamicPalindrome moved = std::move(copy);
        assert(moved.size() == reference.size() && copy.size() == 0);
        assert(copy.is_palindrome(0, 0));
        copy = moved;
        moved = std::move(text);
        assert(text.size() == 0);
        assert(copy.is_palindrome(0, n) == moved.is_palindrome(0, n));
        if (n > 0)
        {
            copy.set(0, static_cast<std::uint8_t>(copy.get(0) + 1));
            assert(copy.get(0) != moved.get(0));
        }
    }

    constexpr std::size_t kLength = 1'000'000;
    sirkit::DynamicPalindrome large(std::string(kLength, 'a'), 296);
    for (std::size_t position = 0; position < 5000; position += 5)
    {
        large.set(position, 'b');
        assert(!large.is_palindrome(0, kLength));
        large.set(kLength - 1 - position, 'b');
        assert(large.is_palindrome(0, kLength));
    }
    expect_throw<std::length_error>([] {
        (void)sirkit::detail::dynamic_palindrome_nodes(std::numeric_limits<std::size_t>::max());
    });
#ifndef SKIP_RUNTIME_CHECKS
    expect_throw<std::out_of_range>([&] { (void)empty.get(0); });
    expect_throw<std::out_of_range>([&] { large.set(large.size(), 'a'); });
    expect_throw<std::out_of_range>([&] { (void)large.is_palindrome(6, 4); });
    expect_throw<std::out_of_range>([&] { (void)large.is_palindrome(0, large.size() + 1); });
#endif
}
