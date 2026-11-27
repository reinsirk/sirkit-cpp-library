// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_PALINDROMIC_FACTORIZATION_DEFINED
#define SIRKIT_PALINDROMIC_FACTORIZATION_REFERENCE_SOURCE "../../../String/Palindromic_Factorization.cpp"
#include SIRKIT_PALINDROMIC_FACTORIZATION_REFERENCE_SOURCE
#undef SIRKIT_PALINDROMIC_FACTORIZATION_REFERENCE_SOURCE
#endif
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
bool is_palindrome(std::string_view text)
{
    return std::equal(text.begin(), text.begin() + text.size() / 2, text.rbegin());
}

void compare(std::string_view text)
{
    const auto answer = sirkit::min_palindromic_partition(text);
    assert(!answer.empty() && answer.front() == 0 && answer.back() == static_cast<int>(text.size()));
    for (std::size_t i = 1; i < answer.size(); ++i)
    {
        assert(answer[i - 1] < answer[i]);
        assert(is_palindrome(text.substr(answer[i - 1], answer[i] - answer[i - 1])));
    }

    // 列挙した全回文を有向辺とみなして、接頭辞の長さの順に最短距離を求める。
    std::vector<std::size_t> distance(text.size() + 1, text.size());
    distance[0] = 0;
    for (std::size_t start = 0; start < text.size(); ++start)
    {
        for (std::size_t end = start + 1; end <= text.size(); ++end)
        {
            if (is_palindrome(text.substr(start, end - start)))
            {
                distance[end] = std::min(distance[end], distance[start] + 1);
            }
        }
    }
    assert(answer.size() - 1 == distance.back());
}

void enumerate(std::string &text, int remaining, std::string_view alphabet)
{
    compare(text);
    if (remaining == 0)
    {
        return;
    }
    for (const char symbol : alphabet)
    {
        text.push_back(symbol);
        enumerate(text, remaining - 1, alphabet);
        text.pop_back();
    }
}
} // namespace

int main()
{
    std::string text;
    enumerate(text, 14, "ab");
    enumerate(text, 10, std::string_view{"\0\x80\xff", 3});
    std::mt19937 random(1439);
    for (int i = 0; i < 1500; ++i)
    {
        const unsigned alphabet = 1u + random() % 256;
        text.resize(random() % 120);
        for (auto &symbol : text)
        {
            symbol = static_cast<char>(random() % alphabet);
        }
        compare(text);
    }

    constexpr int kN = (1 << 19) - 1;
    text.resize(kN);
    for (unsigned i = 0; i < text.size(); ++i)
    {
        // Zimin word は接尾回文の系列数が大きい。
        text[i] = static_cast<char>(std::countr_zero(i + 1));
    }
    assert(sirkit::min_palindromic_partition(text) == (std::vector<int>{0, kN}));
    text.push_back('\xff');
    assert(sirkit::min_palindromic_partition(text) == (std::vector<int>{0, kN, kN + 1}));
    text.assign(kN, '\0');
    assert(sirkit::min_palindromic_partition(text) == (std::vector<int>{0, kN}));

    for (unsigned i = 0; i < text.size(); ++i)
    {
        // 長さ2以上の回文がない全バイトの周期入力。
        text[i] = static_cast<char>(i % 256);
    }
    const auto separate = sirkit::min_palindromic_partition(text);
    assert(separate.size() == text.size() + 1);
    for (int i = 0; i <= kN; ++i)
    {
        assert(separate[i] == i);
    }

    bool failed = false;
    try
    {
        (void)sirkit::detail::PalindromicFactorizationSize(std::numeric_limits<std::size_t>::max());
    }
    catch (const std::length_error &)
    {
        failed = true;
    }
    assert(failed);
}
