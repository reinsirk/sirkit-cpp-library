// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_PALINDROMES_IN_DEQUE_DEFINED
#include "../../../String/Palindromes_In_Deque.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace
{
using Deque = sirkit::PalindromesInDeque;

void compare(const Deque &deque, std::string_view text)
{
    std::set<std::string_view> palindromes;
    std::size_t prefix = 0, suffix = 0;
    for (std::size_t first = 0; first < text.size(); ++first)
    {
        for (std::size_t length = 1; length <= text.size() - first; ++length)
        {
            const auto word = text.substr(first, length);
            if (std::equal(word.begin(), word.end(), word.rbegin()))
            {
                palindromes.insert(word);
                if (first == 0)
                {
                    prefix = std::max(prefix, length);
                }
                if (first + length == text.size())
                {
                    suffix = std::max(suffix, length);
                }
            }
        }
    }
    assert(deque.size() == text.size());
    assert(deque.empty() == text.empty());
    assert(deque.distinct_palindrome_count() == palindromes.size());
    assert(deque.longest_prefix_length() == prefix);
    assert(deque.longest_suffix_length() == suffix);
}

void apply(Deque &deque, std::string &text, unsigned type, char letter)
{
    switch (type)
    {
    case 0:
        deque.push_front(letter);
        text.insert(text.begin(), letter);
        break;
    case 1:
        deque.push_back(letter);
        text.push_back(letter);
        break;
    case 2:
        deque.pop_front();
        text.erase(text.begin());
        break;
    default:
        deque.pop_back();
        text.pop_back();
        break;
    }
}

void exhaustive(const Deque &base, const std::string &text, unsigned depth)
{
    compare(base, text);
    if (depth == 0)
    {
        return;
    }
    const unsigned count = text.empty() ? 4 : 6;
    for (unsigned action = 0; action < count; ++action)
    {
        auto next = base;
        auto word = text;
        if (action < 4)
        {
            apply(next, word, action / 2, static_cast<char>('a' + action % 2));
        }
        else
        {
            apply(next, word, action - 2, 'a');
        }
        exhaustive(next, word, depth - 1);
    }
}
} // namespace

int main()
{
    exhaustive(Deque(), "", 7);

    std::mt19937 random(724194);
    for (int trial = 0; trial < 120; ++trial)
    {
        const unsigned alphabet = 1 + random() % 26;
        Deque deque;
        std::string text;
        for (int step = 0; step < 500; ++step)
        {
            unsigned type = random() % 4;
            if (text.empty())
            {
                type &= 1;
            }
            else if (text.size() == 24)
            {
                type |= 2;
            }
            apply(deque, text, type, static_cast<char>('a' + random() % alphabet));
            compare(deque, text);
        }
    }

    Deque original;
    original.push_back('b');
    original.push_front('a');
    original.push_back('a');
    Deque copy;
    copy = original;
    compare(copy, "aba");
    original.pop_front();
    compare(original, "ba");
    Deque moved(std::move(copy));
    compare(moved, "aba");
    compare(copy, "");
    copy.push_front('z');
    compare(copy, "z");

    bool threw = false;
    try
    {
        (void)Deque(std::numeric_limits<std::size_t>::max());
    }
    catch (const std::length_error &)
    {
        threw = true;
    }
    assert(threw);

#ifndef SKIP_RUNTIME_CHECKS
    for (const char c : std::string_view("A{*"))
    {
        threw = false;
        try
        {
            original.push_back(c);
        }
        catch (const std::invalid_argument &)
        {
            threw = true;
        }
        assert(threw);
        compare(original, "ba");
    }
    threw = false;
    try
    {
        Deque().pop_back();
    }
    catch (const std::out_of_range &)
    {
        threw = true;
    }
    assert(threw);
#endif

    constexpr std::size_t kUniform = 200'000;
    Deque maximum(500'000);
    for (std::size_t i = 1; i <= kUniform; ++i)
    {
        maximum.push_back('a');
        assert(maximum.distinct_palindrome_count() == i);
        assert(maximum.longest_prefix_length() == i);
        assert(maximum.longest_suffix_length() == i);
    }
    for (std::size_t i = 0; i < 150'000; ++i)
    {
        if (i % 2 == 0)
        {
            maximum.push_front('b');
            assert(maximum.longest_prefix_length() == 1);
            assert(maximum.longest_suffix_length() == kUniform);
            assert(maximum.distinct_palindrome_count() == kUniform + 1);
            maximum.pop_front();
        }
        else
        {
            maximum.push_back('b');
            assert(maximum.longest_prefix_length() == kUniform);
            assert(maximum.longest_suffix_length() == 1);
            assert(maximum.distinct_palindrome_count() == kUniform + 1);
            maximum.pop_back();
        }
        assert(maximum.longest_prefix_length() == kUniform);
        assert(maximum.longest_suffix_length() == kUniform);
        assert(maximum.distinct_palindrome_count() == kUniform);
    }
}
