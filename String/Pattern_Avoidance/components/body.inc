#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_PATTERN_AVOIDANCE_DEFINED
#define SIRKIT_PATTERN_AVOIDANCE_DEFINED

namespace reinsirk::detail
{
template <std::size_t Alphabet, unsigned First>
std::vector<std::array<int, Alphabet>> BuildPatternAvoidanceTransitions(
    std::uint64_t length, std::span<const std::string_view> forbidden)
{
    AhoCorasick<Alphabet, First> automaton;
    for (const auto pattern : forbidden)
    {
        if (pattern.empty())
        {
            return {};
        }
        if (pattern.size() <= length)
        {
            static_cast<void>(automaton.add(pattern));
        }
    }
    automaton.build();

    std::vector<int> vertices{0};
    std::vector<int> index(static_cast<std::size_t>(automaton.state_count()), -1);
    index[0] = 0;
    std::vector<std::array<int, Alphabet>> adjacency;
    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        const int from = vertices[i];
        auto &row = adjacency.emplace_back();
        for (std::size_t letter = 0; letter < Alphabet; ++letter)
        {
            const int to = automaton.step(from, static_cast<unsigned char>(First + letter));
            if (automaton.output_count(to) != 0)
            {
                row[letter] = -1;
                continue;
            }
            auto &dense = index[static_cast<std::size_t>(to)];
            if (dense == -1)
            {
                dense = static_cast<int>(vertices.size());
                vertices.push_back(to);
            }
            row[letter] = dense;
        }
    }
    return adjacency;
}

template <class Field>
void AddPatternAvoidanceCount(Field &sum, const Field &count)
{
    sum += count;
    if constexpr (std::is_same_v<Field, std::uint32_t>)
    {
        constexpr std::uint32_t kMod = 998244353;
        if (sum >= kMod)
        {
            sum -= kMod;
        }
    }
}
} // namespace reinsirk::detail

template <std::size_t Alphabet = 26, unsigned First = 'a', class Field = std::uint32_t>
[[nodiscard]] Field count_strings_avoiding_patterns(std::uint64_t length, std::span<const std::string_view> forbidden)
{
    const auto adjacency = ::reinsirk::detail::BuildPatternAvoidanceTransitions<Alphabet, First>(length, forbidden);
    const auto states = adjacency.size();
    if (states == 0)
    {
        return Field{0};
    }
    const auto terms = length < 2 * states ? static_cast<std::size_t>(length) + 1 : 2 * states;
    std::vector<Field> counts;
    counts.reserve(terms);
    std::vector<Field> current(states, Field{0}), next(states, Field{0});
    current[0] = Field{1};
    for (std::size_t i = 0; i < terms; ++i)
    {
        std::fill(next.begin(), next.end(), Field{0});
        Field total{0};
        for (std::size_t vertex = 0; vertex < states; ++vertex)
        {
            const auto &ways = current[vertex];
            if (ways == Field{0})
            {
                continue;
            }
            ::reinsirk::detail::AddPatternAvoidanceCount(total, ways);
            for (const auto to : adjacency[vertex])
            {
                if (to != -1)
                {
                    ::reinsirk::detail::AddPatternAvoidanceCount(next[static_cast<std::size_t>(to)], ways);
                }
            }
        }
        counts.push_back(total);
        current.swap(next);
    }
    if (length < counts.size())
    {
        return counts[static_cast<std::size_t>(length)];
    }

    // S状態の遷移行列の特性多項式から、個数列の漸化式の次数は高々S。
    const auto recurrence = BerlekampMassey(counts);
    counts.resize(recurrence.size(), Field{0});
    return LinearRecurrenceTerm(counts, recurrence, length);
}

#endif
