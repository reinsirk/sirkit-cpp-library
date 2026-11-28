#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>

#ifndef SIRKIT_THREE_SYMBOL_FREQUENCY_COUNTS_DEFINED
#define SIRKIT_THREE_SYMBOL_FREQUENCY_COUNTS_DEFINED

struct ThreeSymbolFrequencyCounts
{
    std::array<std::uint64_t, 3> equal_pairs{};
    std::uint64_t all_equal = 0;
    std::uint64_t all_distinct = 0;
};

namespace reinsirk::detail
{
inline void CheckThreeSymbolTextSize(std::size_t size)
{
    if (size >= static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
    {
        throw std::length_error("analyze_three_symbol_frequencies: text too long");
    }
}
} // namespace reinsirk::detail

[[nodiscard]] inline ThreeSymbolFrequencyCounts analyze_three_symbol_frequencies(
    std::string_view text, const std::array<char, 3> &symbols = {'A', 'B', 'C'})
{
#ifndef SKIP_RUNTIME_CHECKS
    if (symbols[0] == symbols[1] || symbols[0] == symbols[2] || symbols[1] == symbols[2])
    {
        throw std::invalid_argument("analyze_three_symbol_frequencies: duplicate symbol");
    }
#endif
    const std::size_t n = text.size();
    reinsirk::detail::CheckThreeSymbolTextSize(n);
    std::array<std::vector<std::uint32_t>, 3> pair_frequency;
    for (auto &frequency : pair_frequency)
    {
        frequency.resize(2 * n + 1);
        frequency[n] = 1;
    }
    HashMap<std::uint64_t, std::uint32_t> triple_frequency;
    triple_frequency[0] = 1;

    ThreeSymbolFrequencyCounts result;
    std::array<std::int64_t, 3> prefix{};
    for (const char byte : text)
    {
        if (byte == symbols[0])
        {
            ++prefix[0];
        }
        else if (byte == symbols[1])
        {
            ++prefix[1];
        }
        else if (byte == symbols[2])
        {
            ++prefix[2];
        }

        const std::array differences{
            prefix[0] - prefix[1],
            prefix[0] - prefix[2],
            prefix[1] - prefix[2],
        };
        // 前後の接頭辞で差が同じなら、その間の出現回数も等しい。
        for (std::size_t axis = 0; axis < differences.size(); ++axis)
        {
            const auto slot = static_cast<std::size_t>(differences[axis] + static_cast<std::int64_t>(n));
            result.equal_pairs[axis] += pair_frequency[axis][slot]++;
        }
        const auto first = static_cast<std::uint32_t>(differences[0]);
        const auto second = static_cast<std::uint32_t>(differences[1]);
        const auto key = (static_cast<std::uint64_t>(first) << 32) | second;
        result.all_equal += triple_frequency[key]++;
    }

    const auto total = static_cast<std::uint64_t>(n) * (n + 1) / 2;
    // ちょうど1組が等しい場合と、3つとも等しい場合を足す。
    auto repeated = result.all_equal;
    for (const auto pairs : result.equal_pairs)
    {
        repeated += pairs - result.all_equal;
    }
    result.all_distinct = total - repeated;
    return result;
}

#endif
