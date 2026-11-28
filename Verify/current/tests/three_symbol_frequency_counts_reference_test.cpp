#include "../../../String/Three_Symbol_Frequency_Counts.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
using sirkit::ThreeSymbolFrequencyCounts;
using sirkit::analyze_three_symbol_frequencies;

ThreeSymbolFrequencyCounts brute(std::string_view text, const std::array<char, 3> &symbols)
{
    ThreeSymbolFrequencyCounts result;
    for (std::size_t left = 0; left < text.size(); ++left)
    {
        std::array<unsigned, 3> count{};
        for (std::size_t right = left; right < text.size(); ++right)
        {
            for (std::size_t i = 0; i < symbols.size(); ++i)
            {
                count[i] += text[right] == symbols[i];
            }
            const bool eq01 = count[0] == count[1];
            const bool eq02 = count[0] == count[2];
            const bool eq12 = count[1] == count[2];
            result.equal_pairs[0] += eq01;
            result.equal_pairs[1] += eq02;
            result.equal_pairs[2] += eq12;
            result.all_equal += eq01 && eq02 && eq12;
            result.all_distinct += !eq01 && !eq02 && !eq12;
        }
    }
    return result;
}

void verify(std::string_view text, const std::array<char, 3> &symbols)
{
    const auto expected = brute(text, symbols);
    const auto actual = analyze_three_symbol_frequencies(text, symbols);
    assert(actual.equal_pairs == expected.equal_pairs);
    assert(actual.all_equal == expected.all_equal && actual.all_distinct == expected.all_distinct);
}

template <typename Exception, typename Action>
void expect_exception(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}
}

int main()
{
    constexpr std::array symbols{'A', 'B', 'C'};
    verify("", symbols);
    verify("A", symbols);
    verify("BBBB", symbols);
    verify("AAAAABBBB", symbols);
    verify("ABCABCABC", symbols);
    verify("YXYGXGYYXX!", {'X', 'Y', 'G'});
    verify(std::string_view{"\0\xff\0ab\xff\0", 7}, {'\0', '\xff', 'a'});

    std::mt19937 random(455037);
    for (int trial = 0; trial < 2700; ++trial)
    {
        std::string text(random() % 44, 'A');
        for (auto &byte : text)
        {
            byte = static_cast<char>('A' + random() % 6);
        }
        verify(text, symbols);
        verify(text, {'D', 'A', 'F'});
    }

    constexpr std::uint64_t wide = 5000050000ULL;
    const auto untracked = analyze_three_symbol_frequencies(std::string(100000, '.'));
    assert(untracked.all_equal == wide && untracked.all_distinct == 0);
    assert((untracked.equal_pairs == std::array<std::uint64_t, 3>{wide, wide, wide}));

    expect_exception<std::length_error>([] {
        sirkit::detail::CheckThreeSymbolTextSize(std::numeric_limits<std::int32_t>::max());
    });
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::invalid_argument>([] { (void)analyze_three_symbol_frequencies("A", {'A', 'A', 'B'}); });
    expect_exception<std::invalid_argument>([] { (void)analyze_three_symbol_frequencies("A", {'A', 'B', 'A'}); });
    expect_exception<std::invalid_argument>([] { (void)analyze_three_symbol_frequencies("A", {'A', 'B', 'B'}); });
#endif
}
