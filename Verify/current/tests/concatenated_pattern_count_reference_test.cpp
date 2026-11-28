// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_CONCATENATED_PATTERN_COUNT_DEFINED
#include "../../../String/Concatenated_Pattern_Count.cpp"
#endif
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
std::uint32_t naive_count(std::string_view text, std::string_view pattern, std::uint32_t modulus)
{
    std::size_t matches = 0;
    for (std::size_t start = 0; (start = text.find(pattern, start)) != std::string_view::npos; ++start)
    {
        ++matches;
    }
    return static_cast<std::uint32_t>(matches % modulus);
}

template <class Exception, class Function>
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
    using sirkit::ConcatenatedPatternCounter;
    const auto max_modulus = std::numeric_limits<std::uint32_t>::max();
    std::mt19937 random(2741);
    for (std::uint32_t modulus : {1U, 2U, 7U, 998'244'353U, max_modulus})
    {
        for (int trial = 0; trial < 210; ++trial)
        {
            std::string pattern(1 + random() % 8, '\0');
            std::string text(random() % 38, '\0');
            for (auto &byte : pattern)
            {
                byte = static_cast<char>(random() % 3 * 127);
            }
            for (auto &byte : text)
            {
                byte = static_cast<char>(random() % 3 * 127);
            }
            const ConcatenatedPatternCounter counter(pattern, modulus);
            auto copy = counter;
            copy = counter;
            auto result = counter.summarize("");
            const auto expected = naive_count(text, pattern, modulus);
            for (std::size_t first = 0; first <= text.size();)
            {
                const std::size_t step = 1 + random() % 5;
                result = copy.concatenate(result, counter.summarize(text.substr(first, step)));
                first += step;
            }
            assert(copy.count(result) == expected);
            assert(copy.count(counter.summarize(text)) == expected);
            const auto split = text.size() / 2;
            const auto left = counter.summarize(text.substr(0, split));
            const auto right = copy.summarize(text.substr(split));
            result = left;
            assert(counter.count(counter.concatenate(result, right)) == expected);
        }

        const ConcatenatedPatternCounter power("a", modulus);
        auto summary = power.summarize("a");
        std::uint64_t length_mod = 1 % modulus;
        for (int n = 0; n < 120; ++n)
        {
            assert(power.count(summary) == length_mod);
            summary = power.concatenate(summary, summary);
            length_mod = length_mod * 2 % modulus;
        }
    }

    std::string every_byte;
    for (unsigned value = 0; value < 256; ++value)
    {
        every_byte.push_back(static_cast<char>(value));
    }
    const ConcatenatedPatternCounter full(every_byte);
    const auto full_text = every_byte + every_byte;
    assert(full.count(full.concatenate(full.summarize(every_byte.substr(0, 200)),
                                       full.summarize(full_text.substr(200)))) == 2);

    expect_throw<std::invalid_argument>([] { (void)ConcatenatedPatternCounter(""); });
    expect_throw<std::invalid_argument>([] { (void)ConcatenatedPatternCounter("a", 0); });
    expect_throw<std::length_error>([] { (void)sirkit::detail::concatenated_pattern_table_size(5, 3, 14); });
#ifndef SKIP_RUNTIME_CHECKS
    const ConcatenatedPatternCounter first("aba"), second("aba");
    const auto separate = second.summarize("");
    expect_throw<std::invalid_argument>([&] { (void)first.count(separate); });
    expect_throw<std::invalid_argument>([&] { (void)first.concatenate(first.summarize(""), separate); });
    auto summary = first.summarize("aba");
    const auto moved = std::move(summary);
    assert(first.count(moved) == 1);
    expect_throw<std::invalid_argument>([&] { (void)first.count(summary); });
#endif
}
