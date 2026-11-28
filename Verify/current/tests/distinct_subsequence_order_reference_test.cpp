// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Distinct_Subsequence_Order.cpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

int main()
{
    std::mt19937 rng(931183);
    const std::array alphabet{'\0', 'c', static_cast<char>(0xff), 'a'};
    for (std::size_t trial = 0; trial < 110; ++trial)
    {
        std::string text(trial % 11, '\0');
        for (auto &byte : text)
        {
            byte = alphabet[rng() % alphabet.size()];
        }
        const auto unsigned_lex_less = [](const std::string &first, const std::string &second) {
            return std::lexicographical_compare(first.begin(), first.end(), second.begin(), second.end(),
                                                [](unsigned char a, unsigned char b) { return a < b; });
        };
        std::set<std::string, decltype(unsigned_lex_less)> expected(unsigned_lex_less);
        for (unsigned mask = 1; mask < (1U << text.size()); ++mask)
        {
            std::string needle;
            for (std::size_t i = 0; i < text.size(); ++i)
            {
                if (((mask >> i) & 1U) != 0)
                {
                    needle.push_back(text[i]);
                }
            }
            expected.insert(needle);
        }
        const sirkit::DistinctSubsequenceOrder order(text);
        assert(order.size() == text.size());
        assert(order.count_distinct() == expected.size());
        assert(order.prefix_count("") == expected.size());
        assert(order.prefix_count("absent") == 0);
        assert(!order.rank_of("absent"));
        assert(!order.rank_of(""));
        assert(!order.kth_smallest(0));
        assert(!order.kth_smallest(expected.size() + 1));
        std::uint64_t rank = 0;
        for (const auto &pattern : expected)
        {
            ++rank;
            assert(order.kth_smallest(rank) == pattern);
            assert(order.rank_of(pattern) == rank);
            const auto prefix = std::string_view(pattern).substr(0, pattern.size() / 2 + 1);
            const auto matches = std::ranges::count_if(expected, [&](const auto &other) { return other.starts_with(prefix); });
            assert(order.prefix_count(prefix) == static_cast<std::uint64_t>(matches));
        }
    }

    std::string all_bytes;
    for (unsigned byte = 0; byte < 64; ++byte)
    {
        all_bytes.push_back(static_cast<char>(byte));
    }
    constexpr auto cap = std::numeric_limits<std::uint64_t>::max();
    const sirkit::DistinctSubsequenceOrder exact(all_bytes);
    assert(exact.count_distinct() == cap);
    assert(exact.kth_smallest(cap) == std::string(1, 63));
    all_bytes.push_back(static_cast<char>(64));
    sirkit::DistinctSubsequenceOrder saturated(all_bytes);
    assert(saturated.count_distinct() == cap);
    assert(saturated.kth_smallest(cap) == std::string({char{0}, char{63}, char{64}}));
    assert(saturated.rank_of(std::string(1, 64)) == cap);
    assert(saturated.prefix_count(std::string(1, 0)) == cap);
    const auto shared = saturated;
    const auto moved = std::move(saturated);
    assert(shared.kth_smallest(cap) == moved.kth_smallest(cap));
    assert(saturated.size() == 0);
    assert(saturated.count_distinct() == 0);
    assert(saturated.prefix_count("a") == 0);
    assert(!saturated.rank_of("a"));

    bool too_large = false;
    try
    {
        static_cast<void>(sirkit::detail::subsequence_slot_count(std::vector<std::uint64_t>().max_size()));
    }
    catch (const std::length_error &)
    {
        too_large = true;
    }
    assert(too_large);
}
