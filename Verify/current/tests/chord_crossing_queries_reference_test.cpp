// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/Chord_Crossing_Queries.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <utility>
#include <vector>

using Label = std::int64_t;
using Chord = std::pair<Label, Label>;
using sirkit::count_chord_crossings;

void compare_with_brute(std::span<const Chord> chords, std::span<const Chord> queries)
{
    const auto counts = count_chord_crossings(chords, queries);
    assert(counts.size() == queries.size());
    for (std::size_t j = 0; j < queries.size(); ++j)
    {
        const auto [left, right] = std::minmax(queries[j].first, queries[j].second);
        int hits = 0;
        for (const auto &[a, b] : chords)
        {
            const auto [start, end] = std::minmax(a, b);
            hits += (start < left && left < end && end < right) ||
                    (left < start && start < right && right < end);
        }
        assert(counts[j] == hits);
    }
}

void edge_cases()
{
    assert(count_chord_crossings({}, {}).empty());
    const std::array queries{Chord{10, 4}, Chord{2, 5}, Chord{7, 7}};
    compare_with_brute({}, queries);
    compare_with_brute(queries, {});

    const std::array chords{Chord{0, 3}, Chord{2, 1}, Chord{5, 1}, Chord{4, 4},
                            Chord{6, 3}, Chord{3, 6}, Chord{5, 2}, Chord{3, 5}};
    const std::array touches{Chord{1, 5}, Chord{5, 1}, Chord{0, 3}, Chord{3, 3}};
    compare_with_brute(chords, touches);
    compare_with_brute(chords, chords);

    constexpr Label low = std::numeric_limits<Label>::min(), high = std::numeric_limits<Label>::max();
    constexpr std::array<Label, 6> extremes{low, -3, -1, 0, 1, high};
    std::vector<Chord> all;
    for (const auto a : extremes)
    {
        for (const auto b : extremes)
        {
            all.emplace_back(a, b);
        }
    }
    compare_with_brute(all, all);
}

void random_cases()
{
    std::mt19937 random(0xd0b559);
    for (int test = 0; test < 1800; ++test)
    {
        std::vector<Chord> chords(random() % 52), queries(random() % 40);
        const auto label = [&]()
        {
            return static_cast<Label>(random() % 31) - 15;
        };
        for (auto &[left, right] : chords)
        {
            left = label();
            right = label();
        }
        for (auto &[left, right] : queries)
        {
            left = label();
            right = label();
        }
        compare_with_brute(chords, queries);
    }
}

int main()
{
    edge_cases();
    random_cases();
}
