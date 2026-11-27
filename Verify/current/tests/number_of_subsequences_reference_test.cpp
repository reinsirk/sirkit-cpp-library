// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_COUNT_DISTINCT_SUBSEQUENCES_DEFINED
#include "../../../String/Number_Of_Subsequences.cpp"
#endif
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <set>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace
{
template <class Symbol>
void compare(const std::vector<Symbol> &sequence, std::uint64_t modulus)
{
    std::set<std::vector<Symbol>> subsequences;
    for (std::size_t mask = 1; mask < (std::size_t{1} << sequence.size()); ++mask)
    {
        std::vector<Symbol> selected;
        for (std::size_t i = 0; i < sequence.size(); ++i)
        {
            if ((mask >> i) & 1)
            {
                selected.push_back(sequence[i]);
            }
        }
        subsequences.insert(selected);
    }
    assert(sirkit::count_distinct_subsequences(sequence, modulus) == subsequences.size() % modulus);
}

std::uint64_t pow_two(std::size_t exponent, std::uint64_t mod)
{
    std::uint64_t result = 1;
    for (std::uint64_t base = 2; exponent > 0; exponent /= 2)
    {
        if (exponent % 2 == 1)
        {
            result = result * base % mod;
        }
        base = base * base % mod;
    }
    return result;
}
} // namespace

int main()
{
    constexpr std::uint64_t kModulus = 998'244'353;
    constexpr auto high = std::numeric_limits<std::uint64_t>::max();
    constexpr auto signed_low = std::numeric_limits<std::int64_t>::min();
    constexpr auto signed_high = std::numeric_limits<std::int64_t>::max();

    assert(sirkit::count_distinct_subsequences(std::string_view{}) == 0);
    assert(sirkit::count_distinct_subsequences(std::string_view("aba")) == 6);
    assert(sirkit::count_distinct_subsequences(std::array{1, 2, 1}, 3) == 0);
    assert(sirkit::count_distinct_subsequences(std::array{1, 2, 1}, 1) == 0);
    const std::array<std::int64_t, 3> small{-2, -1, -2};
    assert(sirkit::count_distinct_subsequences(std::span(small)) == 6);
    compare(std::vector<bool>{true, false, true, false}, kModulus);
    compare(std::vector<std::int8_t>{-3, -2, -1, 0, 1, -2, 0}, kModulus);
    compare(std::vector<std::int8_t>{-128, 127, 0, -128, 127}, kModulus);
    compare(std::vector<std::uint64_t>{high, 0, high - 1, high, 0}, high);
    compare(std::vector<std::uint64_t>{high - 1, high, high - 1, high - 1, high}, high);
    compare(std::vector<std::int64_t>{signed_low, signed_high, signed_low + 1, 0, signed_low}, high);

    bool threw = false;
    try
    {
        (void)sirkit::count_distinct_subsequences(small, 0);
    }
    catch (const std::invalid_argument &)
    {
        threw = true;
    }
    assert(threw);

    std::vector<int> wide_mod(64);
    std::iota(wide_mod.begin(), wide_mod.end(), 0);
    assert(sirkit::count_distinct_subsequences(wide_mod, high) == 0);
    wide_mod.push_back(63);
    assert(sirkit::count_distinct_subsequences(wide_mod, high) == (std::uint64_t{1} << 63));
    wide_mod.pop_back();
    wide_mod.pop_back();
    wide_mod.push_back(0);
    assert(sirkit::count_distinct_subsequences(wide_mod, high) == high - 1);

    std::mt19937 random(68739);
    constexpr std::array<std::int64_t, 6> symbols{signed_low, signed_low + 3, -1, 0, 1, signed_high};
    for (int trial = 0; trial < 1300; ++trial)
    {
        std::vector<std::int64_t> sequence(random() % 11);
        for (auto &v : sequence)
        {
            if (trial % 2 == 0)
            {
                v = symbols[random() % symbols.size()];
            }
            else
            {
                v = static_cast<std::int64_t>(random() % 5) - 2;
            }
        }
        compare(sequence, 1 + random() % 29);
    }

    constexpr std::size_t kMaxSize = 500'000;
    std::vector<std::uint64_t> maximum(kMaxSize, high);
    assert(sirkit::count_distinct_subsequences(maximum) == kMaxSize);
    for (std::size_t i = 0; i < maximum.size(); ++i)
    {
        maximum[i] = high - 1'000'000'000'039 * i;
    }
    const auto expected = (pow_two(kMaxSize, kModulus) + kModulus - 1) % kModulus;
    assert(sirkit::count_distinct_subsequences(maximum) == expected);
}
